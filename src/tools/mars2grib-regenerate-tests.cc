/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// Regenerate the expected output of mars2grib test cases.
///
/// For every record the `mars`, `misc` and `opt` objects are encoded with the current library
/// and a new record is written with the freshly generated `out`. Several inputs (block files and
/// JSONL, plain or zstd) can be merged into one output. Work is done in batches: whole blocks for
/// block-file input, groups of records for JSONL input. One thread schedules the batches, N workers
/// read/regenerate/compress them and one thread writes them in input order.

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "eckit/blockfile/Filter.h"
#include "eckit/blockfile/Format.h"
#include "eckit/blockfile/Reader.h"
#include "eckit/blockfile/Writer.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/mars2grib/api/Mars2GribTestCaseGenerator.h"
#include "metkit/mars2grib/testing-utils/BoundedQueue.h"
#include "metkit/mars2grib/testing-utils/JsonlWriter.h"
#include "metkit/mars2grib/testing-utils/Producer.h"
#include "metkit/mars2grib/testing-utils/TestCaseLabels.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"
#include "metkit/mars2grib/testing-utils/ThreadUtils.h"
#include "metkit/tool/MetkitTool.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace {

constexpr std::size_t recordsPerJsonlBatch = 64;
constexpr std::size_t maxReportedErrors    = 10;

struct InputBatch {
    std::size_t index{0};
    /// Block-file input: the reader and block to read. JSONL input: the records themselves.
    const eckit::blockfile::Reader* reader{nullptr};
    std::size_t blockId{0};
    std::vector<std::string> records;
};

struct RecordResult {
    bool regenerated{false};
    /// The regenerated record, or the original one if regeneration failed.
    std::string record;
    std::string error;
    /// Block labels of the regenerated record (block output unless packed by the worker).
    eckit::blockfile::Labels labels;
    /// Identity of the test (hash of its mars, misc and opt), for --deduplicate.
    std::uint64_t identity{0};
};

struct OutputBatch {
    std::size_t index{0};
    std::size_t regenerated{0};
    /// Records dropped by --exclude.
    std::size_t excluded{0};
    /// Failed records always; regenerated records unless they were packed into `block`.
    std::vector<RecordResult> results;
    /// Block output from a block input with the same keys: the records compressed by the worker.
    std::optional<eckit::blockfile::CompressedBlock> block;
};

/// The regenerated record, or nullopt when `exclude` matches its MARS request.
std::optional<std::string> regenerateRecord(const std::string& record, RecordParser parser,
                                            const MarsRecordFilter& exclude, const TestCaseLabeler& labeler,
                                            eckit::blockfile::Labels* labels, std::uint64_t* identity) {
    const eckit::LocalConfiguration root = parseRecord(record, parser);
    const eckit::LocalConfiguration mars = requiredSubConfiguration(root, "mars");
    const eckit::LocalConfiguration misc = optionalSubConfiguration(root, "misc").value_or(eckit::LocalConfiguration{});
    const eckit::LocalConfiguration opt  = optionalSubConfiguration(root, "opt").value_or(eckit::LocalConfiguration{});

    if (!exclude.empty() && exclude.matches(mars)) {
        return std::nullopt;
    }
    if (labels != nullptr) {
        *labels = labeler.labels(mars, misc, opt);
    }
    if (identity != nullptr) {
        *identity = testCaseIdentity(mars, misc, opt);
    }
    Mars2GribTestCaseGenerator generator{opt};
    return generator.generate(mars, misc);
}

bool sameKeys(const std::vector<eckit::blockfile::LabelKey>& a, const std::vector<eckit::blockfile::LabelKey>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].name != b[i].name || a[i].type != b[i].type) {
            return false;
        }
    }
    return true;
}

}  // namespace
}  // namespace metkit::mars2grib::testing_utils::run_tests

namespace {

class Mars2GribRegenerateTestsTool final : public metkit::MetkitTool {
public:

    Mars2GribRegenerateTestsTool(int argc, char** argv) : MetkitTool(argc, argv) {
        using eckit::option::SimpleOption;
        options_.push_back(new SimpleOption<std::string>(
            "input",
            "Input test cases, comma-separated: block files and/or JSONL (zstd-compressed or plain), "
            "read in order and merged into one output"));
        options_.push_back(new SimpleOption<std::string>("output", "Output file"));
        options_.push_back(new SimpleOption<std::string>(
            "output-format",
            "block or jsonl (default: block if any input is a block file, else jsonl; jsonl output is "
            "zstd-compressed if the name ends in .zst)"));
        options_.push_back(
            new SimpleOption<long>("num-threads", "Total threads: 1 scheduler + workers + 1 writer (minimum 3)"));
        options_.push_back(new SimpleOption<long>("max-queue-size", "Maximum queued batches per queue (default 64)"));
        options_.push_back(new SimpleOption<long>("zstd-level", "zstd compression level, 1..22 (default 9)"));
        options_.push_back(
            new SimpleOption<long>("records-per-block", "Block output: records per newly built block (default 1000)"));
        options_.push_back(new SimpleOption<long>(
            "block-bytes", "Block output: uncompressed bytes per newly built block (default 4 MiB)"));
        options_.push_back(new SimpleOption<std::string>(
            "select", "Block-file inputs only: regenerate blocks matching e.g. class=od,stream=enfo/oper,type=pf"));
        options_.push_back(new SimpleOption<std::string>(
            "keys",
            "Block output: comma-separated label keys (default class,stream,type). timeShape is the product "
            "time spec shape type; any other name is a MARS key; absent values are labelled 'missing'"));
        options_.push_back(new SimpleOption<std::string>(
            "exclude",
            "Drop records whose mars matches any rule; rules separated by ';', each key=value[/value...][,...] "
            "with a/to/b ranges and 'missing', e.g. stream=enfo,type=pf;class=rd"));
        options_.push_back(new SimpleOption<std::string>(
            "exclude-file", "File of --exclude rules, one per line ('#' starts a comment)"));
        options_.push_back(new SimpleOption<bool>(
            "deduplicate", "Keep only the first record of every distinct (mars, misc, opt), across all inputs"));
        options_.push_back(new SimpleOption<std::string>(
            "failed-records", "Write the original records that could not be regenerated to this JSONL file"));
        options_.push_back(new SimpleOption<long>(
            "expected-messages", "Total records, only to display progress (known automatically for block inputs)"));
        options_.push_back(new SimpleOption<bool>(
            "yaml-parser", "Parse records with the eckit YAML parser instead of the JSON parser (slower)"));
    }

private:

    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    struct Input {
        std::string path;
        bool blockFile{false};
    };

    std::vector<Input> inputs_;
    std::string outputPath_;
    bool blockOutput_{false};
    bool deduplicate_{false};
    metkit::mars2grib::testing_utils::run_tests::TestCaseLabeler labeler_;
    metkit::mars2grib::testing_utils::run_tests::MarsRecordFilter exclude_;
    std::optional<std::string> select_;
    std::optional<std::string> failedRecordsPath_;
    std::optional<std::size_t> expectedMessages_;
    std::size_t threadCount_{0};
    std::size_t queueSize_{64};
    int zstdLevel_{9};
    std::size_t recordsPerBlock_{1000};
    std::size_t bytesPerBlock_{4 * 1024 * 1024};
    metkit::mars2grib::testing_utils::run_tests::RecordParser parser_{
        metkit::mars2grib::testing_utils::run_tests::RecordParser::Json};
};

long positive(const eckit::option::CmdArgs& args, const std::string& name) {
    long value = 0;
    args.get(name, value);
    if (value <= 0) {
        throw eckit::UserError("--" + name + " must be positive", Here());
    }
    return value;
}

void Mars2GribRegenerateTestsTool::init(const eckit::option::CmdArgs& args) {
    using metkit::mars2grib::testing_utils::run_tests::RecordParser;

    if (!args.has("input") || !args.has("output") || !args.has("num-threads")) {
        throw eckit::UserError("--input, --output and --num-threads are required", Here());
    }
    std::string inputList;
    args.get("input", inputList);
    args.get("output", outputPath_);

    std::vector<std::string> paths;
    eckit::Tokenizer(",")(inputList, paths);
    if (paths.empty()) {
        throw eckit::UserError("--input must name at least one file", Here());
    }
    const std::string output = eckit::PathName{outputPath_}.fullName();
    bool anyBlock            = false;
    bool allBlock            = true;
    for (const auto& path : paths) {
        if (eckit::PathName{path}.fullName() == output) {
            throw eckit::UserError("--output must differ from every --input", Here());
        }
        const bool blockFile = eckit::blockfile::isBlockFile(eckit::PathName{path});
        anyBlock             = anyBlock || blockFile;
        allBlock             = allBlock && blockFile;
        inputs_.push_back(Input{path, blockFile});
    }

    std::string format = anyBlock ? "block" : "jsonl";
    if (args.has("output-format")) {
        args.get("output-format", format);
    }
    if (format != "block" && format != "jsonl") {
        throw eckit::UserError("--output-format must be block or jsonl", Here());
    }
    blockOutput_ = format == "block";
    deduplicate_ = args.getBool("deduplicate", false);
    if (args.has("keys")) {
        std::string keys;
        args.get("keys", keys);
        labeler_ = metkit::mars2grib::testing_utils::run_tests::TestCaseLabeler{keys};
    }
    if (args.has("exclude")) {
        std::string rules;
        args.get("exclude", rules);
        exclude_.addRules(rules);
    }
    if (args.has("exclude-file")) {
        std::string path;
        args.get("exclude-file", path);
        std::ifstream in{path};
        if (!in) {
            throw eckit::UserError("Unable to open --exclude-file `" + path + "`", Here());
        }
        std::ostringstream rules;
        rules << in.rdbuf();
        exclude_.addRules(rules.str());
    }

    const long threadCount = positive(args, "num-threads");
    if (threadCount < 3 || threadCount >= 128) {
        throw eckit::UserError("--num-threads must be in the range 3..127", Here());
    }
    threadCount_ = static_cast<std::size_t>(threadCount);

    if (args.has("max-queue-size")) {
        queueSize_ = static_cast<std::size_t>(positive(args, "max-queue-size"));
    }
    if (args.has("zstd-level")) {
        const long level = positive(args, "zstd-level");
        if (level > 22) {
            throw eckit::UserError("--zstd-level must be in the range 1..22", Here());
        }
        zstdLevel_ = static_cast<int>(level);
    }
    if (args.has("records-per-block")) {
        recordsPerBlock_ = static_cast<std::size_t>(positive(args, "records-per-block"));
    }
    if (args.has("block-bytes")) {
        bytesPerBlock_ = static_cast<std::size_t>(positive(args, "block-bytes"));
    }
    if (args.has("select")) {
        if (!allBlock) {
            throw eckit::UserError("--select needs every input to be a block file", Here());
        }
        std::string select;
        args.get("select", select);
        select_ = std::move(select);
    }
    if (args.has("failed-records")) {
        std::string path;
        args.get("failed-records", path);
        if (path.empty()) {
            throw eckit::UserError("--failed-records must not be empty", Here());
        }
        failedRecordsPath_ = std::move(path);
    }
    if (args.has("expected-messages")) {
        expectedMessages_ = static_cast<std::size_t>(positive(args, "expected-messages"));
    }

    parser_ = args.getBool("yaml-parser", false) ? RecordParser::Yaml : RecordParser::Json;
}

void Mars2GribRegenerateTestsTool::execute(const eckit::option::CmdArgs&) {
    using namespace metkit::mars2grib::testing_utils::run_tests;

    const std::vector<int> cpus = availableCpus(threadCount_);
    const std::size_t workers   = threadCount_ - 2;
    const auto& outputKeys      = labeler_.keys();

    // Open the block inputs up front: their tables give the blocks to schedule and the record count.
    std::vector<std::unique_ptr<eckit::blockfile::Reader>> readers(inputs_.size());
    std::vector<std::vector<std::size_t>> blockIds(inputs_.size());
    std::optional<std::size_t> expected = expectedMessages_;
    std::size_t knownRecords            = 0;
    bool allBlock                       = true;
    for (std::size_t i = 0; i < inputs_.size(); ++i) {
        if (!inputs_[i].blockFile) {
            allBlock = false;
            continue;
        }
        readers[i] = std::make_unique<eckit::blockfile::Reader>(eckit::PathName{inputs_[i].path});
        blockIds[i] =
            readers[i]->select(select_ ? eckit::blockfile::Filter::parse(*select_) : eckit::blockfile::Filter{});
        for (std::size_t id : blockIds[i]) {
            knownRecords += readers[i]->blocks()[id].records;
        }
    }
    if (!expected && allBlock) {
        expected = knownRecords;
    }

    BoundedQueue<InputBatch> input{queueSize_};
    BoundedQueue<OutputBatch> output{queueSize_};
    std::atomic<std::size_t> activeWorkers{workers};
    std::size_t regenerated = 0;
    std::size_t duplicates  = 0;
    std::size_t excluded    = 0;
    std::size_t failures    = 0;
    std::exception_ptr fatalError;
    std::mutex fatalErrorMutex;

    const auto recordFatalError = [&](std::exception_ptr error) {
        {
            std::lock_guard<std::mutex> lock{fatalErrorMutex};
            if (!fatalError) {
                fatalError = std::move(error);
            }
        }
        input.cancel();
        output.cancel();
    };

    std::vector<std::thread> threads;
    threads.reserve(threadCount_);
    try {
        // Scheduler: the inputs in order; block ids for block files, batches of records for JSONL.
        threads.emplace_back([&]() {
            try {
                bindCurrentThread(cpus[0]);
                std::size_t index = 0;
                for (std::size_t i = 0; i < inputs_.size(); ++i) {
                    if (readers[i]) {
                        for (std::size_t id : blockIds[i]) {
                            InputBatch batch;
                            batch.index   = index++;
                            batch.reader  = readers[i].get();
                            batch.blockId = id;
                            if (!input.push(std::move(batch))) {
                                return;
                            }
                        }
                        continue;
                    }

                    Producer producer{inputs_[i].path};
                    InputBatch batch;
                    while (auto record = producer.readNextRecord()) {
                        batch.records.push_back(std::move(*record));
                        if (batch.records.size() == recordsPerJsonlBatch) {
                            batch.index = index++;
                            if (!input.push(std::move(batch))) {
                                return;
                            }
                            batch = InputBatch{};
                        }
                    }
                    if (!batch.records.empty()) {
                        batch.index = index++;
                        if (!input.push(std::move(batch))) {
                            return;
                        }
                    }
                }
                input.close();
            }
            catch (...) {
                recordFatalError(std::current_exception());
            }
        });

        // Workers
        for (std::size_t threadIndex = 1; threadIndex <= workers; ++threadIndex) {
            threads.emplace_back([&, threadIndex]() {
                try {
                    bindCurrentThread(cpus[threadIndex]);
                    while (auto batch = input.pop()) {
                        OutputBatch result;
                        result.index = batch->index;

                        eckit::blockfile::Labels blockLabels;
                        if (batch->reader != nullptr) {
                            const eckit::blockfile::Block block = batch->reader->readBlock(batch->blockId);
                            blockLabels                         = block.info().labels;
                            batch->records.reserve(block.info().records);
                            block.forEachRecord([&](std::string_view record) { batch->records.emplace_back(record); });
                        }

                        // A block can be repacked by the worker when its labels are valid for the output and no
                        // record-level decision (deduplication) is needed; otherwise the writer regroups records.
                        const bool packBlock = blockOutput_ && batch->reader != nullptr && !deduplicate_ &&
                                               !labeler_.hasDerivedKeys() &&
                                               sameKeys(batch->reader->keys(), outputKeys);
                        const bool needLabels = blockOutput_ && !packBlock;
                        std::string packed;
                        std::uint32_t packedRecords = 0;

                        for (auto& record : batch->records) {
                            RecordResult item;
                            try {
                                auto regeneratedRecord = regenerateRecord(record, parser_, exclude_, labeler_,
                                                                          needLabels ? &item.labels : nullptr,
                                                                          deduplicate_ ? &item.identity : nullptr);
                                if (!regeneratedRecord) {
                                    ++result.excluded;
                                    continue;
                                }
                                item.record      = std::move(*regeneratedRecord);
                                item.regenerated = true;
                            }
                            catch (const std::exception& exception) {
                                item.error  = exception.what();
                                item.record = std::move(record);
                            }
                            catch (...) {
                                item.error  = "Unknown exception";
                                item.record = std::move(record);
                            }

                            if (item.regenerated) {
                                ++result.regenerated;
                                if (packBlock) {
                                    packed += item.record;
                                    packed += '\n';
                                    ++packedRecords;
                                    continue;
                                }
                            }
                            result.results.push_back(std::move(item));
                        }

                        if (packedRecords != 0) {
                            result.block =
                                eckit::blockfile::Writer::compressBlock(blockLabels, packed, packedRecords, zstdLevel_);
                        }
                        if (!output.push(std::move(result))) {
                            return;
                        }
                    }
                    if (activeWorkers.fetch_sub(1) == 1) {
                        output.close();
                    }
                }
                catch (...) {
                    recordFatalError(std::current_exception());
                }
            });
        }

        // Writer: restores the input order before writing.
        threads.emplace_back([&]() {
            try {
                bindCurrentThread(cpus[threadCount_ - 1]);

                std::optional<eckit::blockfile::Writer> blockWriter;
                std::optional<JsonlWriter> jsonlWriter;
                if (blockOutput_) {
                    eckit::blockfile::WriterOptions options;
                    options.keys             = outputKeys;
                    options.recordsPerBlock  = recordsPerBlock_;
                    options.bytesPerBlock    = bytesPerBlock_;
                    options.compressionLevel = zstdLevel_;
                    blockWriter.emplace(eckit::PathName{outputPath_}, options);
                }
                else {
                    jsonlWriter.emplace(outputPath_, zstdLevel_);
                }
                std::optional<JsonlWriter> rejected;
                if (failedRecordsPath_) {
                    rejected.emplace(*failedRecordsPath_, zstdLevel_);
                }

                std::unordered_set<std::uint64_t> seen;
                std::map<std::size_t, OutputBatch> pending;
                std::size_t next                = 0;
                std::size_t processed           = 0;
                std::size_t displayedHundredths = 0;

                const auto handle = [&](OutputBatch& batch) {
                    regenerated += batch.regenerated;
                    excluded += batch.excluded;
                    processed += batch.excluded;
                    if (batch.block) {
                        processed += batch.block->records;
                        blockWriter->addCompressedBlock(std::move(*batch.block));
                    }
                    for (auto& item : batch.results) {
                        ++processed;
                        if (!item.regenerated) {
                            if (failures < maxReportedErrors) {
                                std::cerr << "\nFAILED to regenerate a record: " << item.error << std::endl;
                            }
                            ++failures;
                            if (rejected) {
                                rejected->write(item.record);
                            }
                            continue;
                        }
                        if (deduplicate_ && !seen.insert(item.identity).second) {
                            ++duplicates;
                            --regenerated;
                            continue;
                        }
                        if (blockWriter) {
                            blockWriter->add(item.labels, item.record);
                        }
                        else {
                            jsonlWriter->write(item.record);
                        }
                    }

                    if (expected && *expected != 0) {
                        const std::size_t hundredths = static_cast<std::size_t>(
                            10000.0L * static_cast<long double>(processed) / static_cast<long double>(*expected));
                        if (hundredths > displayedHundredths) {
                            displayedHundredths = hundredths;
                            std::cerr << '\r' << "PROGRESS: " << hundredths / 100 << '.' << std::setw(2)
                                      << std::setfill('0') << hundredths % 100 << '%' << std::setfill(' ')
                                      << std::flush;
                        }
                    }
                };

                while (auto batch = output.pop()) {
                    pending.emplace(batch->index, std::move(*batch));
                    for (auto it = pending.find(next); it != pending.end(); it = pending.find(next)) {
                        handle(it->second);
                        pending.erase(it);
                        ++next;
                    }
                }

                {
                    std::lock_guard<std::mutex> lock{fatalErrorMutex};
                    if (fatalError) {
                        return;
                    }
                }
                if (!pending.empty()) {
                    throw eckit::Exception(
                        "Internal error: " + std::to_string(pending.size()) + " batches could not be written in order",
                        Here());
                }
                if (blockWriter) {
                    blockWriter->close();
                }
                if (jsonlWriter) {
                    jsonlWriter->close();
                }
                if (rejected) {
                    rejected->close();
                }
            }
            catch (...) {
                recordFatalError(std::current_exception());
            }
        });
    }
    catch (...) {
        const std::exception_ptr startupError = std::current_exception();
        input.cancel();
        output.cancel();
        for (auto& thread : threads) {
            thread.join();
        }
        rethrowAsEckit(startupError, "Unable to start regeneration threads");
    }

    for (auto& thread : threads) {
        thread.join();
    }

    if (expected) {
        std::cerr << std::endl;
    }

    if (fatalError) {
        rethrowAsEckit(fatalError, "Regeneration thread failed");
    }

    eckit::Log::info() << "INPUTS: " << inputs_.size() << '\n'
                       << "TOT RECORDS: " << regenerated + duplicates + excluded + failures << '\n'
                       << "REGENERATED: " << regenerated << '\n';
    if (!exclude_.empty()) {
        eckit::Log::info() << "EXCLUDED (" << exclude_.numberOfRules() << " rules): " << excluded << '\n';
    }
    if (deduplicate_) {
        eckit::Log::info() << "DUPLICATES (dropped): " << duplicates << '\n';
    }
    eckit::Log::info() << "FAILED: " << failures << '\n'
                       << "OUTPUT: " << outputPath_ << " (" << (blockOutput_ ? "block file" : "JSONL") << ")"
                       << std::endl;
    if (failures != 0) {
        if (failedRecordsPath_) {
            eckit::Log::info() << "FAILED RECORDS: " << *failedRecordsPath_ << std::endl;
        }
        throw eckit::Exception(std::to_string(failures) + " records could not be regenerated", Here());
    }
}

void Mars2GribRegenerateTestsTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool
                       << " --input <file>[,<file>...] --output <file> --num-threads <3..127> "
                          "[--output-format block|jsonl] [--keys <k1,k2,...>] [--exclude <rules>] [--exclude-file "
                          "<file>] [--deduplicate] "
                          "[--select <filter>] [--max-queue-size <N>] "
                          "[--zstd-level <1..22>] [--records-per-block <N>] [--block-bytes <N>] "
                          "[--failed-records <file.jsonl[.zst]>] [--expected-messages <N>] [--yaml-parser]"
                       << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribRegenerateTestsTool tool(argc, argv);
    return tool.start();
}

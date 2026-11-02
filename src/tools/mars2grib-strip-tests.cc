/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// Strip a large set of mars2grib test cases with a configurable plan (MPI).
///
///   mpirun -n <N> mars2grib-strip-tests --plan plan.yaml
///
/// plan.yaml:
///   feed:    { file-list: <text file, one JSONL path per line (plain or .zst)>, round-size: 2000 }
///   actions: [ ... ]   # see metkit/mars2grib/testing-utils/strip/Plan.h
///   sink:    { file: out.ekbf, keys: "class,stream,type,levtype,timeShape",
///              records-per-block: 100, block-bytes: 4194304, zstd-level: 22 }
///
/// Every rank reads its share of the files (file i goes to rank i % N), wraps each record in an
/// Envelope (time representation, first-emission flag, routing hash) and sends it to the rank owning
/// hash(class, stream, type, levtype, anchorType, domainType, shapeType) % N. Each rank runs the plan
/// on the records it owns, in a fixed order (source rank, then sequence), and writes the kept ones to
/// <file>.part-<rank>.ekbf; rank 0 merges the parts into <file> and prints per-action statistics.

#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "eckit/blockfile/Reader.h"
#include "eckit/blockfile/Writer.h"
#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/mpi/Comm.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/mars2grib/testing-utils/Producer.h"
#include "metkit/mars2grib/testing-utils/strip/Envelope.h"
#include "metkit/mars2grib/testing-utils/strip/Plan.h"
#include "metkit/tool/MetkitTool.h"

namespace {

using namespace metkit::mars2grib::testing_utils::strip;
using metkit::mars2grib::testing_utils::run_tests::Producer;

/// Records of a list of files, read one after the other.
class Feed {
public:

    explicit Feed(std::vector<std::string> files) : files_{std::move(files)} {}

    std::optional<std::string> next() {
        while (true) {
            if (current_) {
                if (auto record = current_->readNextRecord()) {
                    return record;
                }
                current_.reset();
            }
            if (nextFile_ >= files_.size()) {
                exhausted_ = true;
                return std::nullopt;
            }
            current_ = std::make_unique<Producer>(files_[nextFile_++]);
        }
    }

    bool exhausted() const { return exhausted_; }

private:

    std::vector<std::string> files_;
    std::size_t nextFile_{0};
    std::unique_ptr<Producer> current_;
    bool exhausted_{false};
};

std::vector<std::string> readFileList(const std::string& path) {
    std::ifstream in{path};
    if (!in) {
        throw eckit::UserError("Unable to open file list `" + path + "`", Here());
    }
    std::vector<std::string> files;
    std::string line;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) {
            line.pop_back();
        }
        if (!line.empty() && line.front() != '#') {
            files.push_back(line);
        }
    }
    return files;
}

std::string marsText(const metkit::datamod::Dict& mars, const std::string& key) {
    const metkit::datamod::Value* value = mars.find(key);
    if (value != nullptr && value->isString() && !value->asString().empty()) {
        return value->asString();
    }
    if (value != nullptr && value->isInt()) {
        return std::to_string(value->asInt());
    }
    return missingValue;
}

class Mars2GribStripTestsTool final : public metkit::MetkitTool {
public:

    Mars2GribStripTestsTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("plan", "YAML stripping plan"));
    }

private:

    int numberOfPositionalArguments() const override { return 0; }

    void init(const eckit::option::CmdArgs& args) override {
        if (!args.has("plan")) {
            throw eckit::UserError("--plan is required", Here());
        }
        args.get("plan", planPath_);
    }

    void execute(const eckit::option::CmdArgs&) override {
        eckit::mpi::Comm& comm = eckit::mpi::comm();
        try {
            run(comm);
        }
        catch (const std::exception& e) {
            std::cerr << "rank " << comm.rank() << ": " << e.what() << std::endl;
            comm.abort(1);
        }
    }

    void usage(const std::string& tool) const override {
        eckit::Log::info() << "Usage: mpirun -n <N> " << tool << " --plan <plan.yaml>" << std::endl;
    }

    void run(eckit::mpi::Comm& comm) {
        const std::size_t rank  = comm.rank();
        const std::size_t ranks = comm.size();

        const eckit::LocalConfiguration config{eckit::YAMLConfiguration{eckit::PathName{planPath_}}};
        const auto feedConfig       = config.getSubConfiguration("feed");
        const auto sinkConfig       = config.getSubConfiguration("sink");
        const std::size_t roundSize = static_cast<std::size_t>(feedConfig.getLong("round-size", 2000));
        const std::string output    = sinkConfig.getString("file");

        Plan plan{config.getSubConfigurations("actions")};

        std::vector<std::string> keys;
        eckit::Tokenizer(",")(sinkConfig.getString("keys", "class,stream,type,levtype,timeShape"), keys);
        eckit::blockfile::WriterOptions writerOptions;
        for (const auto& key : keys) {
            writerOptions.keys.push_back(eckit::blockfile::LabelKey{key, eckit::blockfile::LabelType::String});
        }
        writerOptions.recordsPerBlock  = static_cast<std::size_t>(sinkConfig.getLong("records-per-block", 100));
        writerOptions.bytesPerBlock    = static_cast<std::size_t>(sinkConfig.getLong("block-bytes", 4 * 1024 * 1024));
        writerOptions.compressionLevel = static_cast<int>(sinkConfig.getLong("zstd-level", 22));

        std::vector<std::string> mine;
        const auto files = readFileList(feedConfig.getString("file-list"));
        for (std::size_t i = rank; i < files.size(); i += ranks) {
            mine.push_back(files[i]);
        }
        Feed feed{std::move(mine)};

        const std::string part = output + ".part-" + std::to_string(rank) + ".ekbf";
        eckit::blockfile::Writer writer{eckit::PathName{part}, writerOptions};

        const auto labels = [&](const TestView& test) {
            eckit::blockfile::Labels out;
            for (const auto& key : keys) {
                if (key == "timeShape") {
                    out.emplace_back(test.envelope.shapeType);
                }
                else if (key == "time-representation") {
                    out.emplace_back(test.envelope.timeRepresentation());
                }
                else {
                    out.emplace_back(marsText(test.mars, key));
                }
            }
            return out;
        };

        std::uint64_t read = 0, received = 0, kept = 0, unclassified = 0, rounds = 0;
        while (true) {
            std::vector<std::vector<char>> send(ranks);
            std::vector<std::vector<char>> receive;
            for (std::size_t n = 0; n < roundSize; ++n) {
                auto record = feed.next();
                if (!record) {
                    break;
                }
                const Envelope envelope = makeEnvelope(std::move(*record));
                unclassified += envelope.classified ? 0 : 1;
                appendEnvelope(send[envelope.route % ranks], envelope);
                ++read;
            }
            const int exhausted = feed.exhausted() ? 1 : 0;

            comm.allToAll(send, receive);

            for (const auto& buffer : receive) {  // fixed order: source rank, then sequence
                std::size_t position = 0;
                Envelope envelope;
                while (readEnvelope(buffer, position, envelope)) {
                    ++received;
                    const TestView test = TestView::parse(envelope);
                    if (plan.keep(test)) {
                        writer.add(labels(test), envelope.record);
                        ++kept;
                    }
                }
            }
            ++rounds;

            int allExhausted = 0;
            comm.allReduce(exhausted, allExhausted, eckit::mpi::min());
            if (allExhausted != 0) {
                break;
            }
        }
        writer.close();

        // Statistics, summed over ranks.
        std::vector<long long> local{static_cast<long long>(read), static_cast<long long>(received),
                                     static_cast<long long>(kept), static_cast<long long>(unclassified)};
        for (const auto& action : plan.actions()) {
            local.push_back(static_cast<long long>(action->kept()));
            local.push_back(static_cast<long long>(action->dropped()));
        }
        std::vector<long long> total(local.size(), 0);
        comm.allReduce(local, total, eckit::mpi::sum());
        comm.barrier();

        if (rank == 0) {
            eckit::blockfile::Writer merged{eckit::PathName{output}, writerOptions};
            for (std::size_t r = 0; r < ranks; ++r) {
                const eckit::PathName partPath{output + ".part-" + std::to_string(r) + ".ekbf"};
                {
                    const eckit::blockfile::Reader reader{partPath};
                    for (const auto& info : reader.blocks()) {
                        merged.addCompressedBlock(
                            {info.labels, info.records, info.uncompressedSize, reader.readCompressed(info.id)});
                    }
                }
                partPath.unlink();
            }
            merged.close();

            auto& out = eckit::Log::info();
            out << "STRIP: " << ranks << " ranks, " << rounds << " rounds\n"
                << "  records read:      " << total[0] << '\n'
                << "  records kept:      " << total[2] << " (" << std::fixed << std::setprecision(4)
                << (total[0] == 0 ? 0.0 : 100.0 * static_cast<double>(total[2]) / static_cast<double>(total[0]))
                << "%)\n"
                << "  unclassified time spec: " << total[3] << '\n';
            for (std::size_t i = 0; i < plan.actions().size(); ++i) {
                out << "  [" << i << "] " << plan.actions()[i]->description() << "\n        kept " << total[4 + 2 * i]
                    << ", dropped " << total[5 + 2 * i] << '\n';
            }
            out << "OUTPUT: " << output << " (" << merged.numberOfRecords() << " tests, " << merged.numberOfBlocks()
                << " blocks)" << std::endl;
        }
        comm.barrier();
    }

    std::string planPath_;
};

}  // namespace

int main(int argc, char** argv) {
    int status = 0;
    {
        Mars2GribStripTestsTool tool(argc, argv);
        status = tool.start();
    }
    eckit::mpi::finaliseAllComms();
    return status;
}

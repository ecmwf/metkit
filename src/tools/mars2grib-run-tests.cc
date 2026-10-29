/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "eckit/blockfile/Filter.h"
#include "eckit/blockfile/Format.h"
#include "eckit/blockfile/Reader.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/BoundedQueue.h"
#include "metkit/mars2grib/testing-utils/Consumer.h"
#include "metkit/mars2grib/testing-utils/Producer.h"
#include "metkit/mars2grib/testing-utils/RunTestsOptions.h"
#include "metkit/mars2grib/testing-utils/ThreadUtils.h"
#include "metkit/tool/MetkitTool.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace {

std::string failurePath(const std::string& prefix, std::size_t threadIndex) {
    return prefix + "." + std::to_string(threadIndex) + ".txt";
}

void printSummary(std::size_t total, std::size_t passed, std::size_t failed) {
    const double passedPercent = total == 0 ? 0.0 : 100.0 * static_cast<double>(passed) / static_cast<double>(total);
    const double failedPercent = total == 0 ? 0.0 : 100.0 * static_cast<double>(failed) / static_cast<double>(total);
    eckit::Log::info() << "TOT TESTS: " << total << '\n'
                       << "PASSED: " << passed << ", " << std::fixed << std::setprecision(2) << passedPercent << "%\n"
                       << "FAILED: " << failed << ", " << failedPercent << '%' << std::endl;
}

}  // namespace
}  // namespace metkit::mars2grib::testing_utils::run_tests

namespace {

using metkit::mars2grib::testing_utils::run_tests::CommonRunTestsOptions;

class Mars2GribRunTestsTool final : public metkit::MetkitTool {
public:

    Mars2GribRunTestsTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "test-cases", "Test-case block file, or JSONL file (zstd-compressed or plain)"));
        options_.push_back(new eckit::option::SimpleOption<long>(
            "num-threads", "Total threads (JSONL input: including the reader thread)"));
        options_.push_back(
            new eckit::option::SimpleOption<long>("max-queue-size", "Maximum queued JSONL records (JSONL input only)"));
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "select", "Block files only: run blocks matching e.g. class=od,stream=enfo/oper,type=pf"));
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "blocks", "Block files only: run these blocks, e.g. 0,3,5-7 (combined with --select if both are given)"));
        options_.push_back(new eckit::option::SimpleOption<long>(
            "expected-messages", "Estimated total messages used only to display progress"));
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "failed-tests", "Prefix for per-consumer files containing failed JSONL records"));
        metkit::mars2grib::testing_utils::run_tests::addCommonRunTestsOptions(options_);
    }

private:

    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    void executeBlocks();
    void executeJsonl();

    std::string archivePath_;
    bool blockFile_{false};
    std::optional<std::string> select_;
    std::optional<std::string> blocks_;
    std::optional<std::string> failedTestsPrefix_;
    std::optional<std::size_t> expectedMessages_;
    std::size_t threadCount_{0};
    std::size_t queueSize_{0};
    CommonRunTestsOptions common_;
};

void Mars2GribRunTestsTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("test-cases") || !args.has("num-threads")) {
        throw eckit::UserError("--test-cases and --num-threads are required", Here());
    }

    args.get("test-cases", archivePath_);
    blockFile_ = eckit::blockfile::isBlockFile(eckit::PathName{archivePath_});

    long threadCount = 0;
    args.get("num-threads", threadCount);
    const long minThreads = blockFile_ ? 1 : 2;
    if (threadCount < minThreads || threadCount >= 128) {
        throw eckit::UserError("--num-threads must be in the range " + std::to_string(minThreads) + "..127", Here());
    }
    threadCount_ = static_cast<std::size_t>(threadCount);

    if (!blockFile_) {
        if (!args.has("max-queue-size")) {
            throw eckit::UserError("--max-queue-size is required for JSONL input", Here());
        }
        long queueSize = 0;
        args.get("max-queue-size", queueSize);
        if (queueSize <= 0) {
            throw eckit::UserError("--max-queue-size must be positive", Here());
        }
        queueSize_ = static_cast<std::size_t>(queueSize);
    }

    if (args.has("select")) {
        if (!blockFile_) {
            throw eckit::UserError("--select needs a block-file input", Here());
        }
        std::string select;
        args.get("select", select);
        select_ = std::move(select);
    }
    if (args.has("blocks")) {
        if (!blockFile_) {
            throw eckit::UserError("--blocks needs a block-file input", Here());
        }
        std::string blocks;
        args.get("blocks", blocks);
        blocks_ = std::move(blocks);
    }

    if (args.has("failed-tests")) {
        std::string prefix;
        args.get("failed-tests", prefix);
        if (prefix.empty()) {
            throw eckit::UserError("--failed-tests must not be empty", Here());
        }
        failedTestsPrefix_ = std::move(prefix);
    }

    if (args.has("expected-messages")) {
        long expectedMessages = 0;
        args.get("expected-messages", expectedMessages);
        if (expectedMessages <= 0) {
            throw eckit::UserError("--expected-messages must be positive", Here());
        }
        expectedMessages_ = static_cast<std::size_t>(expectedMessages);
    }

    common_ = metkit::mars2grib::testing_utils::run_tests::readCommonRunTestsOptions(args);
}

void Mars2GribRunTestsTool::execute(const eckit::option::CmdArgs&) {
    if (blockFile_) {
        executeBlocks();
    }
    else {
        executeJsonl();
    }
}

void Mars2GribRunTestsTool::executeJsonl() {
    using namespace metkit::mars2grib::testing_utils::run_tests;
    using Clock    = std::chrono::steady_clock;
    using Duration = Clock::duration;

    const auto start            = Clock::now();
    const std::vector<int> cpus = availableCpus(threadCount_);
    BoundedQueue<std::string> queue{queueSize_};
    std::atomic<std::size_t> passed{0};
    std::atomic<std::size_t> failed{0};
    std::atomic<std::size_t> completed{0};
    std::atomic<std::size_t> displayedHundredths{0};
    std::mutex progressMutex;
    std::exception_ptr fatalError;
    std::mutex fatalErrorMutex;

    // Each thread writes only its own slot; the slots are read after join().
    Duration readerRead{};
    Duration readerBlocked{};
    std::vector<ConsumerTimings> workerTimings(threadCount_);
    std::vector<Duration> workerWait(threadCount_);

    const auto recordFatalError = [&](std::exception_ptr error) {
        {
            std::lock_guard<std::mutex> lock{fatalErrorMutex};
            if (!fatalError) {
                fatalError = std::move(error);
            }
        }
        queue.cancel();
    };

    const auto updateProgress = [&]() {
        const std::size_t current = completed.fetch_add(1) + 1;
        if (!expectedMessages_) {
            return;
        }

        const std::size_t hundredths = static_cast<std::size_t>(10000.0L * static_cast<long double>(current) /
                                                                static_cast<long double>(*expectedMessages_));
        std::size_t displayed        = displayedHundredths.load();
        while (displayed < hundredths && !displayedHundredths.compare_exchange_weak(displayed, hundredths)) {}
        if (displayed >= hundredths) {
            return;
        }

        std::lock_guard<std::mutex> lock{progressMutex};
        if (displayedHundredths.load() != hundredths) {
            return;
        }
        std::cerr << '\r' << "PROGRESS: " << hundredths / 100 << '.' << std::setw(2) << std::setfill('0')
                  << hundredths % 100 << '%' << std::setfill(' ') << std::flush;
    };

    std::vector<std::thread> threads;
    threads.reserve(threadCount_);
    try {
        threads.emplace_back([&]() {
            try {
                bindCurrentThread(cpus[0]);
                Producer producer{archivePath_};
                auto now = Clock::now();
                while (auto record = producer.readNextRecord()) {
                    const auto read = Clock::now();
                    readerRead += read - now;
                    const bool accepted = queue.push(std::move(*record));
                    now                 = Clock::now();
                    readerBlocked += now - read;
                    if (!accepted) {
                        return;
                    }
                }
                readerRead += Clock::now() - now;
                queue.close();
            }
            catch (...) {
                recordFatalError(std::current_exception());
            }
        });

        for (std::size_t threadIndex = 1; threadIndex < threadCount_; ++threadIndex) {
            threads.emplace_back([&, threadIndex]() {
                try {
                    bindCurrentThread(cpus[threadIndex]);
                    ConsumerOptions options;
                    options.logErrorDetails = false;
                    options.parser          = common_.parser;
                    options.mode            = common_.mode;
                    options.decodeMarsOnly  = common_.decodeMarsOnly;
                    options.failureReports  = common_.failureReports;
                    if (failedTestsPrefix_) {
                        options.failedTestsPath = failurePath(*failedTestsPrefix_, threadIndex);
                    }
                    Consumer consumer{std::move(options)};
                    Duration wait{};
                    while (true) {
                        const auto waitStart = Clock::now();
                        auto record          = queue.pop();
                        wait += Clock::now() - waitStart;
                        if (!record) {
                            break;
                        }
                        if (consumer.run(*record)) {
                            ++passed;
                        }
                        else {
                            ++failed;
                        }
                        updateProgress();
                    }
                    workerTimings[threadIndex] = consumer.timings();
                    workerWait[threadIndex]    = wait;
                }
                catch (...) {
                    recordFatalError(std::current_exception());
                }
            });
        }
    }
    catch (...) {
        const std::exception_ptr startupError = std::current_exception();
        queue.cancel();
        for (auto& thread : threads) {
            thread.join();
        }
        rethrowAsEckit(startupError, "Unable to start test runner threads");
    }

    for (auto& thread : threads) {
        thread.join();
    }
    const Duration wall = Clock::now() - start;

    if (fatalError) {
        if (expectedMessages_) {
            std::cerr << std::endl;
        }
        rethrowAsEckit(fatalError, "Test runner thread failed");
    }

    if (expectedMessages_) {
        std::cerr << std::endl;
    }

    const std::size_t passedCount = passed.load();
    const std::size_t failedCount = failed.load();
    printSummary(passedCount + failedCount, passedCount, failedCount);

    if (common_.failureReports && failedCount != 0) {
        eckit::Log::info() << "Failure reports written under " << common_.failureReports->directory << std::endl;
    }

    if (common_.timings) {
        ConsumerTimings total;
        Duration totalWait{};
        for (std::size_t i = 1; i < threadCount_; ++i) {
            total += workerTimings[i];
            totalWait += workerWait[i];
        }
        printConsumerTimings(eckit::Log::info(), total, passedCount + failedCount, threadCount_ - 1, totalWait, wall);
        eckit::Log::info() << std::fixed << std::setprecision(3) << "  reader [s]: read+decompress "
                           << seconds(readerRead) << " | blocked on full queue " << seconds(readerBlocked)
                           << std::defaultfloat << std::endl;
    }

    if (failedCount != 0) {
        throw eckit::Exception(std::to_string(failedCount) + " mars2grib tests failed", Here());
    }
}

void Mars2GribRunTestsTool::executeBlocks() {
    using namespace metkit::mars2grib::testing_utils::run_tests;
    using Clock    = std::chrono::steady_clock;
    using Duration = Clock::duration;

    const auto start = Clock::now();
    const eckit::blockfile::Reader reader{eckit::PathName{archivePath_}};
    const auto filter = select_ ? eckit::blockfile::Filter::parse(*select_) : eckit::blockfile::Filter{};
    std::vector<std::size_t> ids;
    if (blocks_) {
        for (std::size_t id : parseBlockIds(*blocks_, reader.numberOfBlocks())) {
            if (filter.matches(reader.keys(), reader.blocks()[id])) {
                ids.push_back(id);
            }
        }
    }
    else {
        ids = reader.select(filter);
    }

    std::size_t selectedRecords = 0;
    for (std::size_t id : ids) {
        selectedRecords += reader.blocks()[id].records;
    }
    // The block table gives the exact number of tests: show progress unless told otherwise.
    const std::size_t expected = expectedMessages_.value_or(selectedRecords);

    const std::vector<int> cpus = availableCpus(threadCount_);
    std::atomic<std::size_t> nextBlock{0};
    std::atomic<std::size_t> passed{0};
    std::atomic<std::size_t> failed{0};
    std::atomic<std::size_t> completed{0};
    std::atomic<std::size_t> displayedHundredths{0};
    std::atomic<bool> stop{false};
    std::mutex progressMutex;
    std::exception_ptr fatalError;
    std::mutex fatalErrorMutex;

    // Each thread writes only its own slot; the slots are read after join().
    std::vector<ConsumerTimings> workerTimings(threadCount_);
    std::vector<Duration> workerRead(threadCount_);

    const auto updateProgress = [&]() {
        const std::size_t current = completed.fetch_add(1) + 1;
        if (expected == 0) {
            return;
        }
        const std::size_t hundredths =
            static_cast<std::size_t>(10000.0L * static_cast<long double>(current) / static_cast<long double>(expected));
        std::size_t displayed = displayedHundredths.load();
        while (displayed < hundredths && !displayedHundredths.compare_exchange_weak(displayed, hundredths)) {}
        if (displayed >= hundredths) {
            return;
        }
        std::lock_guard<std::mutex> lock{progressMutex};
        if (displayedHundredths.load() != hundredths) {
            return;
        }
        std::cerr << '\r' << "PROGRESS: " << hundredths / 100 << '.' << std::setw(2) << std::setfill('0')
                  << hundredths % 100 << '%' << std::setfill(' ') << std::flush;
    };

    std::vector<std::thread> threads;
    threads.reserve(threadCount_);
    try {
        for (std::size_t threadIndex = 0; threadIndex < threadCount_; ++threadIndex) {
            threads.emplace_back([&, threadIndex]() {
                try {
                    bindCurrentThread(cpus[threadIndex]);
                    ConsumerOptions options;
                    options.logErrorDetails = false;
                    options.parser          = common_.parser;
                    options.mode            = common_.mode;
                    options.decodeMarsOnly  = common_.decodeMarsOnly;
                    options.failureReports  = common_.failureReports;
                    if (failedTestsPrefix_) {
                        options.failedTestsPath = failurePath(*failedTestsPrefix_, threadIndex);
                    }
                    Consumer consumer{std::move(options)};
                    Duration read{};
                    std::string record;
                    while (!stop.load()) {
                        const std::size_t index = nextBlock.fetch_add(1);
                        if (index >= ids.size()) {
                            break;
                        }
                        const auto readStart                = Clock::now();
                        const eckit::blockfile::Block block = reader.readBlock(ids[index]);
                        read += Clock::now() - readStart;
                        block.forEachRecord([&](std::string_view view) {
                            record.assign(view.data(), view.size());
                            if (consumer.run(record)) {
                                ++passed;
                            }
                            else {
                                ++failed;
                            }
                            updateProgress();
                        });
                    }
                    workerTimings[threadIndex] = consumer.timings();
                    workerRead[threadIndex]    = read;
                }
                catch (...) {
                    {
                        std::lock_guard<std::mutex> lock{fatalErrorMutex};
                        if (!fatalError) {
                            fatalError = std::current_exception();
                        }
                    }
                    stop = true;
                }
            });
        }
    }
    catch (...) {
        const std::exception_ptr startupError = std::current_exception();
        stop                                  = true;
        for (auto& thread : threads) {
            thread.join();
        }
        rethrowAsEckit(startupError, "Unable to start test runner threads");
    }

    for (auto& thread : threads) {
        thread.join();
    }
    const Duration wall = Clock::now() - start;

    if (expected != 0) {
        std::cerr << std::endl;
    }
    if (fatalError) {
        rethrowAsEckit(fatalError, "Test runner thread failed");
    }

    const std::size_t passedCount = passed.load();
    const std::size_t failedCount = failed.load();
    eckit::Log::info() << "BLOCKS: " << ids.size() << " of " << reader.numberOfBlocks()
                       << (select_ ? " (select: " + *select_ + ")" : std::string{})
                       << (blocks_ ? " (blocks: " + *blocks_ + ")" : std::string{}) << std::endl;
    printSummary(passedCount + failedCount, passedCount, failedCount);

    if (common_.failureReports && failedCount != 0) {
        eckit::Log::info() << "Failure reports written under " << common_.failureReports->directory << std::endl;
    }

    if (common_.timings) {
        ConsumerTimings total;
        Duration totalRead{};
        for (std::size_t i = 0; i < threadCount_; ++i) {
            total += workerTimings[i];
            totalRead += workerRead[i];
        }
        printConsumerTimings(eckit::Log::info(), total, passedCount + failedCount, threadCount_, totalRead, wall);
        eckit::Log::info() << "  (block input: wait-input is the time workers spent reading and decompressing blocks)"
                           << std::endl;
    }

    if (failedCount != 0) {
        throw eckit::Exception(std::to_string(failedCount) + " mars2grib tests failed", Here());
    }
}

void Mars2GribRunTestsTool::usage(const std::string& tool) const {
    eckit::Log::info()
        << "Usage: " << tool
        << " --test-cases <file> --num-threads <N> [--max-queue-size <N> (JSONL input)] "
           "[--select class=..,stream=..,type=..] [--blocks <ids>] (block input) [--expected-messages <N>] "
           "[--failed-tests <prefix>] [--failure-reports [--failure-reports-dir <dir>] "
           "[--failure-reports-schema <schema>]] [--mode encode|decode|roundtrip] [--decode-mars-only] [--timings] "
           "[--yaml-parser]"
        << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribRunTestsTool tool(argc, argv);
    return tool.start();
}

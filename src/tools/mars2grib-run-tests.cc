/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <pthread.h>
#include <sched.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/Consumer.h"
#include "metkit/mars2grib/testing-utils/Producer.h"
#include "metkit/tool/MetkitTool.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace {

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_{capacity} {}

    bool push(T value) {
        std::unique_lock<std::mutex> lock{mutex_};
        notFull_.wait(lock, [&]() { return queue_.size() < capacity_ || cancelled_; });
        if (cancelled_) {
            return false;
        }
        queue_.push_back(std::move(value));
        notEmpty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock{mutex_};
        notEmpty_.wait(lock, [&]() { return !queue_.empty() || closed_ || cancelled_; });
        if (queue_.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop_front();
        notFull_.notify_one();
        return value;
    }

    void close() {
        std::lock_guard<std::mutex> lock{mutex_};
        closed_ = true;
        notEmpty_.notify_all();
    }

    void cancel() {
        std::lock_guard<std::mutex> lock{mutex_};
        cancelled_ = true;
        queue_.clear();
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

private:
    const std::size_t capacity_;
    std::deque<T> queue_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    bool closed_{false};
    bool cancelled_{false};
};

std::vector<int> availableCpus(std::size_t count) {
    cpu_set_t available;
    CPU_ZERO(&available);
    if (pthread_getaffinity_np(pthread_self(), sizeof(available), &available) != 0) {
        throw eckit::Exception("Unable to obtain process CPU affinity", Here());
    }

    std::vector<int> cpus;
    cpus.reserve(count);
    for (int cpu = 0; cpu < CPU_SETSIZE && cpus.size() < count; ++cpu) {
        if (CPU_ISSET(cpu, &available)) {
            cpus.push_back(cpu);
        }
    }
    if (cpus.size() != count) {
        throw eckit::UserError("Requested " + std::to_string(count) + " threads, but only " +
                                   std::to_string(cpus.size()) + " CPUs are available",
                               Here());
    }
    return cpus;
}

void bindCurrentThread(int cpu) {
    cpu_set_t affinity;
    CPU_ZERO(&affinity);
    CPU_SET(cpu, &affinity);
    if (pthread_setaffinity_np(pthread_self(), sizeof(affinity), &affinity) != 0) {
        throw eckit::Exception("Unable to bind thread to CPU " + std::to_string(cpu), Here());
    }
}

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

[[noreturn]] void rethrowAsEckit(std::exception_ptr error, const std::string& context) {
    try {
        std::rethrow_exception(error);
    }
    catch (const eckit::Exception&) {
        throw;
    }
    catch (const std::exception& exception) {
        throw eckit::Exception(context + ": " + exception.what(), Here());
    }
    catch (...) {
        throw eckit::Exception(context + ": unknown exception", Here());
    }
}

}  // namespace
}  // namespace metkit::mars2grib::testing_utils::run_tests

namespace {

using metkit::mars2grib::testing_utils::run_tests::BoundedQueue;
using metkit::mars2grib::testing_utils::run_tests::Consumer;
using metkit::mars2grib::testing_utils::run_tests::ConsumerOptions;
using metkit::mars2grib::testing_utils::run_tests::Producer;

class Mars2GribRunTestsTool final : public metkit::MetkitTool {
public:
    Mars2GribRunTestsTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("test-cases", "Zstd-compressed JSONL archive"));
        options_.push_back(new eckit::option::SimpleOption<long>("num-threads", "Total threads, including producer"));
        options_.push_back(new eckit::option::SimpleOption<long>("max-queue-size", "Maximum queued JSONL records"));
        options_.push_back(new eckit::option::SimpleOption<long>(
            "expected-messages", "Estimated total messages used only to display progress"));
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "failed-tests", "Prefix for per-consumer files containing failed JSONL records"));
    }

private:
    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    std::string archivePath_;
    std::optional<std::string> failedTestsPrefix_;
    std::optional<std::size_t> expectedMessages_;
    std::size_t threadCount_{0};
    std::size_t queueSize_{0};
};

void Mars2GribRunTestsTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("test-cases") || !args.has("num-threads") || !args.has("max-queue-size")) {
        throw eckit::UserError("--test-cases, --num-threads and --max-queue-size are required", Here());
    }

    args.get("test-cases", archivePath_);
    long threadCount = 0;
    long queueSize   = 0;
    args.get("num-threads", threadCount);
    args.get("max-queue-size", queueSize);
    if (threadCount < 2 || threadCount >= 128) {
        throw eckit::UserError("--num-threads must be in the range 2..127", Here());
    }
    if (queueSize <= 0) {
        throw eckit::UserError("--max-queue-size must be positive", Here());
    }
    threadCount_ = static_cast<std::size_t>(threadCount);
    queueSize_   = static_cast<std::size_t>(queueSize);

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
}

void Mars2GribRunTestsTool::execute(const eckit::option::CmdArgs&) {
    using namespace metkit::mars2grib::testing_utils::run_tests;

    const std::vector<int> cpus = availableCpus(threadCount_);
    BoundedQueue<std::string> queue{queueSize_};
    std::atomic<std::size_t> passed{0};
    std::atomic<std::size_t> failed{0};
    std::atomic<std::size_t> completed{0};
    std::atomic<std::size_t> displayedHundredths{0};
    std::mutex progressMutex;
    std::exception_ptr fatalError;
    std::mutex fatalErrorMutex;

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

        const std::size_t hundredths = static_cast<std::size_t>(
            10000.0L * static_cast<long double>(current) / static_cast<long double>(*expectedMessages_));
        std::size_t displayed = displayedHundredths.load();
        while (displayed < hundredths && !displayedHundredths.compare_exchange_weak(displayed, hundredths)) {
        }
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
                while (auto record = producer.readNextRecord()) {
                    if (!queue.push(std::move(*record))) {
                        return;
                    }
                }
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
                    if (failedTestsPrefix_) {
                        options.failedTestsPath = failurePath(*failedTestsPrefix_, threadIndex);
                    }
                    Consumer consumer{std::move(options)};
                    while (auto record = queue.pop()) {
                        if (consumer.run(*record)) {
                            ++passed;
                        }
                        else {
                            ++failed;
                        }
                        updateProgress();
                    }
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
    if (failedCount != 0) {
        throw eckit::Exception(std::to_string(failedCount) + " mars2grib tests failed", Here());
    }
}

void Mars2GribRunTestsTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool
                       << " --test-cases <archive.zst> --num-threads <2..127> --max-queue-size <N> "
                          "[--expected-messages <N>] [--failed-tests <prefix>]"
                       << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribRunTestsTool tool(argc, argv);
    return tool.start();
}

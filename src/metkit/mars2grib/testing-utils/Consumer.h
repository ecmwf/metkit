/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2grib/testing-utils/Decode.h"
#include "metkit/mars2grib/testing-utils/FailureReport.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

struct ConsumerOptions {
    bool logErrorDetails{false};
    /// Append the JSONL record of every failed test to this file (re-runnable with the scalar tool).
    std::optional<std::string> failedTestsPath;
    /// Write one human-readable report per failed test.
    std::optional<FailureReportOptions> failureReports;
    RecordParser parser{RecordParser::Json};
    TestMode mode{TestMode::Encode};
    /// Decode and roundtrip modes: compare only the decoded mars, not misc.
    bool decodeMarsOnly{false};
};

/// Time spent in each stage of `Consumer::run`, summed over all calls.
struct ConsumerTimings {
    using Duration = std::chrono::steady_clock::duration;

    Duration parse{};
    Duration encode{};
    Duration compare{};
    /// Replay on a GRIB handle, grib2mars and comparison (decode and roundtrip modes).
    Duration decode{};
    Duration failureOutput{};

    ConsumerTimings& operator+=(const ConsumerTimings& other) {
        parse += other.parse;
        encode += other.encode;
        compare += other.compare;
        decode += other.decode;
        failureOutput += other.failureOutput;
        return *this;
    }
};

namespace detail {

struct TestCase {
    eckit::LocalConfiguration mars;
    eckit::LocalConfiguration misc;
    eckit::LocalConfiguration options;
    RecordingDictionary expectedOutput;
};

struct ComparisonResult {
    bool equal{true};
    std::string path;
    std::string reason;
    std::string expected;
    std::string actual;
};

TestCase parseTestCase(std::string_view json, RecordParser parser = RecordParser::Json);
RecordingDictionary generateActual(const TestCase& testCase);
ComparisonResult compareOutputs(const RecordingDictionary& expected, const RecordingDictionary& actual);

}  // namespace detail

class Consumer {
public:

    explicit Consumer(ConsumerOptions options);
    ~Consumer();

    bool run(const std::string& json);

    const ConsumerTimings& timings() const { return timings_; }

private:

    /// What a failed run knows; pointers are null for stages that did not run.
    struct Failure {
        std::string reason;
        bool error{false};
        const detail::TestCase* testCase{nullptr};
        const RecordingDictionary* actual{nullptr};
        const DecodedTestCase* decoded{nullptr};
        std::string decodeDiff;
    };

    bool failed(const std::string& json, const Failure& failure);
    void writeFailureReport(const std::string& json, const Failure& failure);

    ConsumerOptions options_;
    std::ofstream failedTests_;
    std::size_t failedTestsWritten_{0};
    std::unique_ptr<FailureReporter> reporter_;
    ConsumerTimings timings_;
};

}  // namespace metkit::mars2grib::testing_utils::run_tests

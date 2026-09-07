/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"

namespace metkit::mars2grib::testing_utils::run_tests {

struct ConsumerOptions {
    bool logErrorDetails{false};
    std::optional<std::string> failedTestsPath;
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

TestCase parseTestCase(std::string_view json);
RecordingDictionary generateActual(const TestCase& testCase);
ComparisonResult compareOutputs(const RecordingDictionary& expected, const RecordingDictionary& actual);

}  // namespace detail

class Consumer {
public:
    explicit Consumer(ConsumerOptions options);

    bool run(const std::string& json);

private:
    bool failed(const std::string& json, const std::string& reason);

    ConsumerOptions options_;
    std::ofstream failedTests_;
};

}  // namespace metkit::mars2grib::testing_utils::run_tests

/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/Consumer.h"

#include <exception>
#include <iostream>
#include <utility>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/api/Mars2GribTestCaseGenerator.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace detail {

namespace {

eckit::LocalConfiguration requiredSubConfiguration(const eckit::LocalConfiguration& root, const std::string& key) {
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        throw eckit::UserError("Test-case record requires object `" + key + "`", Here());
    }
    return root.getSubConfiguration(key);
}

}  // namespace

TestCase parseTestCase(std::string_view json) {
    const eckit::LocalConfiguration root{eckit::YAMLConfiguration{std::string{json}}};
    return TestCase{requiredSubConfiguration(root, "mars"), requiredSubConfiguration(root, "misc"),
                    requiredSubConfiguration(root, "opt"), RecordingDictionary{requiredSubConfiguration(root, "out")}};
}

RecordingDictionary generateActual(const TestCase& testCase) {
    Mars2GribTestCaseGenerator generator{testCase.options};
    return generator.generateOutput(testCase.mars, testCase.misc);
}

ComparisonResult compareOutputs(const RecordingDictionary& expected, const RecordingDictionary& actual) {
    const auto comparison = expected.compare(actual);
    ComparisonResult result;
    result.equal = comparison.equal;
    if (!comparison.equal) {
        result.path     = comparison.path;
        result.reason   = comparison.reason;
        result.expected = comparison.lhs_json;
        result.actual   = comparison.rhs_json;
    }
    return result;
}

}  // namespace detail

Consumer::Consumer(ConsumerOptions options) : options_{std::move(options)} {
    if (options_.failedTestsPath) {
        failedTests_.open(*options_.failedTestsPath, std::ios::out | std::ios::trunc);
        if (!failedTests_) {
            throw eckit::UserError("Unable to open failed-test file `" + *options_.failedTestsPath + "`", Here());
        }
    }
}

bool Consumer::run(const std::string& json) {
    std::string failureReason;
    try {
        const detail::TestCase testCase = detail::parseTestCase(json);
        const auto actual               = detail::generateActual(testCase);
        const auto comparison           = detail::compareOutputs(testCase.expectedOutput, actual);
        if (!comparison.equal) {
            failureReason = "Output mismatch at " + comparison.path + ": " + comparison.reason +
                            "\nEXPECTED: " + comparison.expected + "\nACTUAL: " + comparison.actual;
        }
    }
    catch (const eckit::Exception& exception) {
        failureReason = exception.what();
    }
    catch (const std::exception& exception) {
        failureReason = exception.what();
    }
    catch (...) {
        failureReason = "Unknown exception";
    }

    return failureReason.empty() ? true : failed(json, failureReason);
}

bool Consumer::failed(const std::string& json, const std::string& reason) {
    if (options_.logErrorDetails) {
        std::cerr << "FAILED: " << reason << '\n' << "RECORD: " << json << std::endl;
    }

    if (failedTests_.is_open()) {
        failedTests_ << json << '\n';
        if (!failedTests_) {
            throw eckit::Exception("Unable to write failed-test record to `" + *options_.failedTestsPath + "`", Here());
        }
    }
    return false;
}

}  // namespace metkit::mars2grib::testing_utils::run_tests

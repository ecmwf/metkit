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
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace detail {

namespace {

template <class Cntx_t>
eckit::LocalConfiguration requiredSubConfiguration(const eckit::LocalConfiguration& root, const std::string& key,
                                                    Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        throw eckit::UserError("Test-case record requires object `" + key + "`", Here());
    }
    eckit::LocalConfiguration result = root.getSubConfiguration(key);
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
TestCase parseTestCaseImpl(std::string_view json, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const eckit::LocalConfiguration root{eckit::YAMLConfiguration{std::string{json}}};
    TestCase result{requiredSubConfiguration(root, "mars", utils::profiling::callSite(cntx, Here())),
                    requiredSubConfiguration(root, "misc", utils::profiling::callSite(cntx, Here())),
                    requiredSubConfiguration(root, "opt", utils::profiling::callSite(cntx, Here())),
                    RecordingDictionary{
                        requiredSubConfiguration(root, "out", utils::profiling::callSite(cntx, Here()))}};
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
RecordingDictionary generateActualImpl(const TestCase& testCase, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    Mars2GribTestCaseGenerator generator{testCase.options};
    RecordingDictionary result = generator.generateOutput(testCase.mars, testCase.misc);
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
ComparisonResult compareOutputsImpl(const RecordingDictionary& expected, const RecordingDictionary& actual,
                                    Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const auto comparison = expected.compare(actual);
    ComparisonResult result;
    result.equal = comparison.equal;
    if (!comparison.equal) {
        result.path     = comparison.path;
        result.reason   = comparison.reason;
        result.expected = comparison.lhs_json;
        result.actual   = comparison.rhs_json;
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
void initializeConsumer(const ConsumerOptions& options, std::ofstream& failedTests, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    if (options.failedTestsPath) {
        failedTests.open(*options.failedTestsPath, std::ios::out | std::ios::trunc);
        if (!failedTests) {
            throw eckit::UserError("Unable to open failed-test file `" + *options.failedTestsPath + "`", Here());
        }
    }
    utils::profiling::profileExitFunction(cntx, Here());
}

template <class Cntx_t>
bool failed(const ConsumerOptions& options, std::ofstream& failedTests, const std::string& json,
            const std::string& reason, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    if (options.logErrorDetails) {
        std::cerr << "FAILED: " << reason << '\n' << "RECORD: " << json << std::endl;
    }

    if (failedTests.is_open()) {
        failedTests << json << '\n';
        if (!failedTests) {
            throw eckit::Exception("Unable to write failed-test record to `" + *options.failedTestsPath + "`", Here());
        }
    }
    const bool result = false;
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace

TestCase parseTestCase(std::string_view json) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    TestCase result = parseTestCaseImpl(json, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

RecordingDictionary generateActual(const TestCase& testCase) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    RecordingDictionary result = generateActualImpl(testCase, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

ComparisonResult compareOutputs(const RecordingDictionary& expected, const RecordingDictionary& actual) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    ComparisonResult result = compareOutputsImpl(expected, actual, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace detail

Consumer::Consumer(ConsumerOptions options) : options_{std::move(options)} {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    detail::initializeConsumer(options_, failedTests_, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
}

bool Consumer::run(const std::string& json) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    std::string failureReason;
    try {
        const detail::TestCase testCase =
            detail::parseTestCaseImpl(json, utils::profiling::callSite(cntx, Here()));
        const auto actual = detail::generateActualImpl(testCase, utils::profiling::callSite(cntx, Here()));
        const auto comparison = detail::compareOutputsImpl(testCase.expectedOutput, actual,
                                                           utils::profiling::callSite(cntx, Here()));
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

    const bool result = failureReason.empty()
                            ? true
                            : detail::failed(options_, failedTests_, json, failureReason,
                                             utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::testing_utils::run_tests

/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/Consumer.h"

#include <exception>
#include <iostream>
#include <utility>

#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/api/Mars2GribTestCaseGenerator.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace detail {

TestCase parseTestCase(std::string_view json, RecordParser parser) {
    const eckit::LocalConfiguration root = parseRecord(json, parser);
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
    if (options_.failureReports) {
        reporter_ = std::make_unique<FailureReporter>(*options_.failureReports);
    }
}

Consumer::~Consumer() = default;

bool Consumer::run(const std::string& json) {
    using Clock = std::chrono::steady_clock;

    // Charge the elapsed time to the stage in progress, then move on to `next`.
    auto stageStart                  = Clock::now();
    ConsumerTimings::Duration* stage = &timings_.parse;
    const auto nextStage             = [&](ConsumerTimings::Duration* next) {
        const auto now = Clock::now();
        *stage += now - stageStart;
        stageStart = now;
        stage      = next;
    };

    const bool encode = options_.mode != TestMode::Decode;
    const bool decode = options_.mode != TestMode::Encode;

    std::optional<detail::TestCase> testCase;
    std::optional<RecordingDictionary> actual;
    std::optional<DecodedTestCase> decoded;
    Failure failure;
    std::string decodeStage;
    try {
        testCase.emplace(detail::parseTestCase(json, options_.parser));

        if (encode) {
            nextStage(&timings_.encode);
            actual.emplace(detail::generateActual(*testCase));
            nextStage(&timings_.compare);
            const auto comparison = detail::compareOutputs(testCase->expectedOutput, *actual);
            if (!comparison.equal) {
                failure.reason = "Output mismatch at " + comparison.path + ": " + comparison.reason +
                                 "\nEXPECTED: " + comparison.expected + "\nACTUAL: " + comparison.actual;
            }
        }

        if (decode) {
            nextStage(&timings_.decode);
            decodeStage = "decode";
            // Decode mode checks the expected output; roundtrip decodes what was just encoded.
            const RecordingDictionary& recording = encode ? *actual : testCase->expectedOutput;
            decoded.emplace(decodeRecording(recording, testCase->options));
            const auto comparison = compareDecoded(testCase->mars, testCase->misc, *decoded);
            if (!comparison.equal) {
                failure.decodeDiff = comparison.diff;
                failure.reason +=
                    (failure.reason.empty() ? "" : "\n") + std::string{"Decode mismatch:\n"} + comparison.diff;
            }
        }
        nextStage(&timings_.failureOutput);
        if (failure.reason.empty()) {
            return true;
        }
    }
    catch (const std::exception& exception) {
        nextStage(&timings_.failureOutput);
        failure.reason += (failure.reason.empty() ? "" : "\n") +
                          (decodeStage.empty() ? std::string{} : decodeStage + " failed: ") + exception.what();
        failure.error = true;
    }
    catch (...) {
        nextStage(&timings_.failureOutput);
        failure.reason += (failure.reason.empty() ? "" : "\n") + std::string{"Unknown exception"};
        failure.error = true;
    }

    failure.testCase  = testCase ? &*testCase : nullptr;
    failure.actual    = actual ? &*actual : nullptr;
    failure.decoded   = decoded ? &*decoded : nullptr;
    const bool result = failed(json, failure);
    nextStage(&timings_.failureOutput);
    return result;
}

bool Consumer::failed(const std::string& json, const Failure& failure) {
    if (options_.logErrorDetails) {
        std::cerr << "FAILED: " << failure.reason << '\n' << "RECORD: " << json << std::endl;
    }

    if (failedTests_.is_open()) {
        failedTests_ << json << '\n';
        if (!failedTests_) {
            throw eckit::Exception("Unable to write failed-test record to `" + *options_.failedTestsPath + "`", Here());
        }
    }

    if (reporter_) {
        writeFailureReport(json, failure);
    }
    return false;
}

void Consumer::writeFailureReport(const std::string& json, const Failure& failure) {
    FailureDetails details;
    details.record     = json;
    details.reason     = failure.reason;
    details.error      = failure.error;
    details.actual     = failure.actual;
    details.mode       = options_.mode;
    details.decodeDiff = failure.decodeDiff;
    if (failure.decoded != nullptr) {
        details.decodedMars = &failure.decoded->mars;
        details.decodedMisc = &failure.decoded->misc;
    }

    if (failure.testCase != nullptr) {
        details.mars     = &failure.testCase->mars;
        details.misc     = &failure.testCase->misc;
        details.options  = &failure.testCase->options;
        details.expected = &failure.testCase->expectedOutput;
        reporter_->write(details);
        return;
    }

    // The record could not be fully parsed: recover whatever parts are readable.
    std::optional<eckit::LocalConfiguration> mars;
    std::optional<eckit::LocalConfiguration> misc;
    std::optional<eckit::LocalConfiguration> options;
    std::optional<RecordingDictionary> expected;
    try {
        const eckit::LocalConfiguration root = parseRecord(json, options_.parser);
        mars                                 = optionalSubConfiguration(root, "mars");
        misc                                 = optionalSubConfiguration(root, "misc");
        options                              = optionalSubConfiguration(root, "opt");
        if (auto out = optionalSubConfiguration(root, "out")) {
            expected.emplace(*out);
        }
    }
    catch (...) {
    }
    details.mars     = mars ? &*mars : nullptr;
    details.misc     = misc ? &*misc : nullptr;
    details.options  = options ? &*options : nullptr;
    details.expected = expected ? &*expected : nullptr;
    reporter_->write(details);
}

}  // namespace metkit::mars2grib::testing_utils::run_tests

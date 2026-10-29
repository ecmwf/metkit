/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Option.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/Consumer.h"
#include "metkit/mars2grib/testing-utils/FailureReport.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// Command-line options shared by `mars2grib-run-tests` and `mars2grib-run-tests-scalar`.
struct CommonRunTestsOptions {
    std::optional<FailureReportOptions> failureReports;
    RecordParser parser{RecordParser::Json};
    TestMode mode{TestMode::Encode};
    bool decodeMarsOnly{false};
    bool timings{false};
};

inline void addCommonRunTestsOptions(std::vector<eckit::option::Option*>& options) {
    options.push_back(
        new eckit::option::SimpleOption<bool>("failure-reports", "Write one text report per failed test"));
    options.push_back(new eckit::option::SimpleOption<std::string>(
        "failure-reports-dir",
        std::string{"Directory for failure reports (default: "} + defaultFailureReportDirectory + ")"));
    options.push_back(new eckit::option::SimpleOption<std::string>(
        "failure-reports-schema",
        std::string{"Report path relative to --failure-reports-dir. {key} is a MARS key, {hash} a hash of the "
                    "record, {status} mismatch/error (default: "} +
            defaultFailureReportSchema + ")"));
    options.push_back(new eckit::option::SimpleOption<bool>(
        "yaml-parser", "Parse records with the eckit YAML parser instead of the JSON parser (slower)"));
    options.push_back(new eckit::option::SimpleOption<bool>("timings", "Print where the time was spent"));
    options.push_back(new eckit::option::SimpleOption<std::string>(
        "mode",
        "encode (default): mars/misc/opt -> set-list vs out; decode: out -> GRIB -> grib2mars vs "
        "mars/misc; roundtrip: encode check, then decode of the encoded output"));
    options.push_back(new eckit::option::SimpleOption<bool>(
        "decode-mars-only", "decode/roundtrip: compare only the decoded mars with the input (ignore misc)"));
}

inline CommonRunTestsOptions readCommonRunTestsOptions(const eckit::option::CmdArgs& args) {
    CommonRunTestsOptions result;
    result.parser         = args.getBool("yaml-parser", false) ? RecordParser::Yaml : RecordParser::Json;
    result.timings        = args.getBool("timings", false);
    result.decodeMarsOnly = args.getBool("decode-mars-only", false);
    if (args.has("mode")) {
        std::string mode;
        args.get("mode", mode);
        result.mode = testModeFromString(mode);
    }

    const bool reports = args.getBool("failure-reports", false);
    if (!reports) {
        if (args.has("failure-reports-dir") || args.has("failure-reports-schema")) {
            throw eckit::UserError("--failure-reports-dir and --failure-reports-schema require --failure-reports",
                                   Here());
        }
        return result;
    }

    FailureReportOptions reportOptions;
    if (args.has("failure-reports-dir")) {
        args.get("failure-reports-dir", reportOptions.directory);
    }
    if (args.has("failure-reports-schema")) {
        args.get("failure-reports-schema", reportOptions.schema);
    }
    if (reportOptions.directory.empty()) {
        throw eckit::UserError("--failure-reports-dir must not be empty", Here());
    }
    FailureReporter::validateSchema(reportOptions.schema);
    result.failureReports = std::move(reportOptions);
    return result;
}

inline double seconds(std::chrono::steady_clock::duration duration) {
    return std::chrono::duration<double>(duration).count();
}

/// Print the per-stage breakdown of the test workers.
///
/// `workers` is the number of consumer threads, `wait` the time they spent waiting for input
/// and `wall` the elapsed time of the whole run.
inline void printConsumerTimings(std::ostream& out, const ConsumerTimings& timings, std::size_t tests,
                                 std::size_t workers, std::chrono::steady_clock::duration wait,
                                 std::chrono::steady_clock::duration wall) {
    const double perTest = tests == 0 ? 0.0 : 1000.0 / static_cast<double>(tests);
    const double busy    = seconds(timings.parse) + seconds(timings.encode) + seconds(timings.compare) +
                        seconds(timings.decode) + seconds(timings.failureOutput);

    out << std::fixed << std::setprecision(3);
    out << "TIMINGS: wall " << seconds(wall) << " s, " << workers << " worker(s), " << tests << " tests, "
        << (seconds(wall) > 0.0 ? static_cast<double>(tests) / seconds(wall) : 0.0) << " tests/s\n";
    out << "  worker time, summed over workers [s]:"
        << " wait-input " << seconds(wait) << " | parse " << seconds(timings.parse) << " | encode "
        << seconds(timings.encode) << " | compare " << seconds(timings.compare) << " | decode "
        << seconds(timings.decode) << " | failure-output " << seconds(timings.failureOutput) << '\n';
    out << "  per test [ms]: parse " << seconds(timings.parse) * perTest << " | encode "
        << seconds(timings.encode) * perTest << " | compare " << seconds(timings.compare) * perTest << " | decode "
        << seconds(timings.decode) * perTest << " | failure-output " << seconds(timings.failureOutput) * perTest
        << '\n';
    if (workers != 0 && seconds(wall) > 0.0) {
        out << "  worker utilisation: " << std::setprecision(1)
            << 100.0 * busy / (static_cast<double>(workers) * seconds(wall)) << "%\n";
    }
    out << std::defaultfloat << std::setprecision(6);
}

}  // namespace metkit::mars2grib::testing_utils::run_tests

/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

inline constexpr const char* defaultFailureReportDirectory = "mars2grib-failure-reports";
inline constexpr const char* defaultFailureReportSchema =
    "{class}-{stream}-{type}-{expver}/{levtype}-{param}-{hash}.txt";

/// Where and how per-test failure reports are written.
///
/// `schema` is a path relative to `directory`. `{key}` is replaced by the value of MARS key `key`
/// (`none` when absent); `{hash}` by a hash of the JSONL record and `{status}` by `mismatch` or
/// `error`. Substituted values are sanitised so they cannot introduce path separators.
struct FailureReportOptions {
    std::string directory{defaultFailureReportDirectory};
    std::string schema{defaultFailureReportSchema};
};

/// Everything known about a failed test. Pointers are null when the information is unavailable
/// (e.g. the record could not be parsed, or encoding threw).
struct FailureDetails {
    std::string_view record;
    std::string_view reason;
    bool error{false};
    const eckit::LocalConfiguration* mars{nullptr};
    const eckit::LocalConfiguration* misc{nullptr};
    const eckit::LocalConfiguration* options{nullptr};
    const RecordingDictionary* expected{nullptr};
    const RecordingDictionary* actual{nullptr};
    TestMode mode{TestMode::Encode};
    /// Decode and roundtrip modes: what grib2mars returned and the per-key differences.
    const eckit::LocalConfiguration* decodedMars{nullptr};
    const eckit::LocalConfiguration* decodedMisc{nullptr};
    std::string_view decodeDiff;
};

/// Writes one human-readable text file per failed test. Not thread-safe: use one per thread.
class FailureReporter {
public:

    explicit FailureReporter(FailureReportOptions options);

    /// Write the report and return its path.
    std::string write(const FailureDetails& details);

    /// Throw `eckit::UserError` if `schema` is not a valid relative path schema.
    static void validateSchema(const std::string& schema);

private:

    struct Token {
        bool placeholder{false};
        std::string text;
    };

    static std::vector<Token> tokenize(const std::string& schema);
    std::string resolvePath(const FailureDetails& details) const;
    void ensureDirectory(const std::string& directory);

    FailureReportOptions options_;
    std::vector<Token> tokens_;
    std::unordered_set<std::string> createdDirectories_;
};

namespace detail {

std::string prettyJson(std::string_view json);
std::string formatOperations(const RecordingDictionary& dictionary);
std::string diffOutputs(const RecordingDictionary& expected, const RecordingDictionary& actual);
std::string recordHash(std::string_view record);

}  // namespace detail

}  // namespace metkit::mars2grib::testing_utils::run_tests

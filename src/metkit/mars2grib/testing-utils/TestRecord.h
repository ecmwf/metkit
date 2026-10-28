/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <vector>

#include "eckit/blockfile/Format.h"
#include "eckit/config/LocalConfiguration.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// Parser used to decode a JSONL test-case record.
///
/// Records are JSON, so `Json` is the default. `Yaml` is the original (slower) path and is kept
/// to compare results and timings.
enum class RecordParser {
    Json,
    Yaml,
};

/// What a test checks.
///  - Encode:    (mars, misc, opt) -> encode -> set-list, compared with `out`.
///  - Decode:    `out` replayed on a GRIB handle -> grib2mars -> (mars, misc), compared with the input.
///  - Roundtrip: Encode, then Decode of the freshly encoded set-list; both checks must pass.
enum class TestMode {
    Encode,
    Decode,
    Roundtrip,
};

TestMode testModeFromString(const std::string& name);
std::string toString(TestMode mode);

eckit::LocalConfiguration parseRecord(std::string_view json, RecordParser parser);

eckit::LocalConfiguration requiredSubConfiguration(const eckit::LocalConfiguration& root, const std::string& key);

std::optional<eckit::LocalConfiguration> optionalSubConfiguration(const eckit::LocalConfiguration& root,
                                                                  const std::string& key);

/// Compact JSON serialisation of a configuration.
std::string configurationToJson(const eckit::LocalConfiguration& configuration);

/// Selects test cases by their MARS request only (not misc or opt).
///
/// Rules are separated by ';' or new lines ('#' starts a comment); each rule is
/// `key=value[/value...][,key=...]`. A record matches when it matches any rule, and a rule
/// matches when every key matches: one of the values, an inclusive numeric range `a/to/b`, or
/// `missing` for an absent key. Strings compare case-insensitively, numbers by value ("130" equals
/// 130); a list value matches when any of its elements does.
class MarsRecordFilter {
public:

    MarsRecordFilter() = default;

    void addRules(const std::string& text);

    bool empty() const { return rules_.empty(); }
    std::size_t numberOfRules() const { return rules_.size(); }

    bool matches(const eckit::LocalConfiguration& mars) const;

private:

    struct Condition {
        std::string key;
        bool missing{false};
        std::vector<std::string> values;
        std::vector<std::pair<double, double>> ranges;
    };

    std::vector<std::vector<Condition>> rules_;
};

/// Parse block ids such as "0,3,5-7" (sorted, without duplicates); each must be below `numberOfBlocks`.
std::vector<std::size_t> parseBlockIds(const std::string& expression, std::size_t numberOfBlocks);


}  // namespace metkit::mars2grib::testing_utils::run_tests

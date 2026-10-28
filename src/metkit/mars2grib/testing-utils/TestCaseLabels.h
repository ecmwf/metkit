/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "eckit/blockfile/Format.h"
#include "eckit/config/LocalConfiguration.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// Label value used for a key that a test case does not have (or cannot be classified).
inline constexpr const char* missingLabel = "missing";

/// Computes the block labels of test cases.
///
/// Keys are given as a comma-separated list. `timeShape` is derived: the shape type of the
/// product time spec (e.g. Instant, IFSStandardSingleLoop, SeasonalMultiloop) computed by
/// Mars2GribClassify. Every other name is the MARS key of that name. All labels are strings;
/// a missing key, a non-scalar value or a failed classification gives `missing`, so blocks can
/// be selected with e.g. `levtype=missing`.
class TestCaseLabeler {
public:

    explicit TestCaseLabeler(const std::string& keys = "class,stream,type");

    const std::vector<eckit::blockfile::LabelKey>& keys() const { return keys_; }

    /// True if some label is computed by mars2grib (it may change when the code changes).
    bool hasDerivedKeys() const { return derived_; }

    eckit::blockfile::Labels labels(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                                    const eckit::LocalConfiguration& opt) const;

private:

    std::vector<eckit::blockfile::LabelKey> keys_;
    bool derived_{false};
};

/// 64-bit FNV-1a, optionally continuing from a previous hash.
std::uint64_t fnv1a(std::string_view text, std::uint64_t hash = 14695981039346656037ULL);

/// Identity of a test case: a hash of its mars, misc and opt (key order does not matter).
std::uint64_t testCaseIdentity(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                               const eckit::LocalConfiguration& opt);

}  // namespace metkit::mars2grib::testing_utils::run_tests

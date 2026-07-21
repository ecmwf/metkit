/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file parseHelpers.h
/// @brief Shared lexical helpers for temporal deductions.
///
/// This header contains small parsing and normalization helpers shared by
/// multiple temporal deductions. These helpers perform only local lexical or
/// arithmetic work; they do not own deduction-level dictionary access,
/// classification, or cross-field semantics.
///

#pragma once

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>

#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions::detail {

using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

inline std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

///
/// @brief Remove one allowed separator character from a token.
///
/// This helper is used by deductions that accept compact and separator-based
/// source syntaxes for the same underlying value.
///
/// @param[in] value Source token.
/// @param[in] ignored Separator character to remove.
/// @return `value` with every occurrence of `ignored` removed.
///
inline std::string digitsOnly(std::string value, char ignored) {
    value.erase(std::remove(value.begin(), value.end(), ignored), value.end());
    return value;
}

///
/// @brief Parse one complete decimal integer token as `long`.
///
/// Partial parses are rejected. The whole token must be consumed by the integer
/// conversion.
///
/// @param[in] value Input token.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Parsed integer value.
/// @throws Mars2GribDeductionException if the token is not a valid complete
///         `long` representation.
///
inline long parseLongStrict(const std::string& value, const std::string& key) {
    std::size_t used = 0;
    long result = 0;

    try {
        result = std::stol(value, &used);
    } catch (...) {
        throw Mars2GribDeductionException("Invalid integer value for `" + key + "`: '" + value + "'", Here());
    }

    if (used != value.size()) {
        throw Mars2GribDeductionException("Invalid trailing characters in `" + key + "`: '" + value + "'", Here());
    }

    return result;
}

///
/// @brief Convert hours to seconds with overflow checking.
///
/// @param[in] hours Hour count.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return `hours * 3600`.
/// @throws Mars2GribDeductionException if the multiplication overflows `long`.
///
inline long checkedHoursToSeconds(long hours, const std::string& key) {
    if (hours > std::numeric_limits<long>::max() / 3600L ||
        hours < std::numeric_limits<long>::min() / 3600L) {
        throw Mars2GribDeductionException("Duration overflow while converting `" + key + "` from hours", Here());
    }

    return hours * 3600L;
}

///
/// @brief Parse a compact duration string and normalize it to seconds.
///
/// Accepted suffixes are:
/// - `h`: hours;
/// - `m`: minutes;
/// - `s`: seconds;
/// - `d`: fixed 86400-second days.
///
/// When no suffix is present, hours are assumed.
///
/// @param[in] raw Duration token.
/// @param[in] key Human-readable source-key name used in diagnostics.
/// @return Duration in seconds.
/// @throws Mars2GribDeductionException for empty input, invalid integer syntax,
///         unsupported units, or arithmetic overflow.
///
inline long parseDurationStringSeconds(const std::string& raw, const std::string& key) {
    if (raw.empty()) {
        throw Mars2GribDeductionException("Empty duration for `" + key + "`", Here());
    }

    std::string value = lower(raw);
    char unit = 'h';
    if (std::isalpha(static_cast<unsigned char>(value.back()))) {
        unit = value.back();
        value.pop_back();
    }

    const long count = parseLongStrict(value, key);
    switch (unit) {
        case 'h':
            return checkedHoursToSeconds(count, key);
        case 'm':
            if (count > std::numeric_limits<long>::max() / 60L ||
                count < std::numeric_limits<long>::min() / 60L) {
                throw Mars2GribDeductionException("Duration overflow in `" + key + "`", Here());
            }
            return count * 60L;
        case 's':
            return count;
        case 'd':
            if (count > std::numeric_limits<long>::max() / 86400L ||
                count < std::numeric_limits<long>::min() / 86400L) {
                throw Mars2GribDeductionException("Duration overflow in `" + key + "`", Here());
            }
            return count * 86400L;
        default:
            throw Mars2GribDeductionException("Unsupported duration unit in `" + key + "`: '" + raw + "'", Here());
    }
}

}  // namespace metkit::mars2grib::backend::deductions::detail

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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>

#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions::detail {

using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

///
/// @brief Convert one token to lowercase using the C locale character mapping.
///
/// This helper is used by deductions that accept case-insensitive textual
/// enumerations. It performs no semantic validation and preserves every
/// non-alphabetic character unchanged.
///
/// @param[in] value Source token.
/// @return Lowercased token.
///
template <class Cntx_t>
inline std::string lower(std::string value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    {
        std::string result = value;
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
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
template <class Cntx_t>
inline std::string digitsOnly(std::string value, char ignored, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    value.erase(std::remove(value.begin(), value.end(), ignored), value.end());
    {
        std::string result = value;
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
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
template <class Cntx_t>
inline long parseLongStrict(const std::string& value, const std::string& key, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    std::size_t used = 0;
    long result      = 0;

    try {
        result = std::stol(value, &used);
    }
    catch (...) {
        throw Mars2GribDeductionException("Invalid integer value for `" + key + "`: '" + value + "'", Here());
    }

    if (used != value.size()) {
        throw Mars2GribDeductionException("Invalid trailing characters in `" + key + "`: '" + value + "'", Here());
    }

    metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
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
template <class Cntx_t>
inline long checkedHoursToSeconds(long hours, const std::string& key, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    if (hours > std::numeric_limits<long>::max() / 3600L || hours < std::numeric_limits<long>::min() / 3600L) {
        throw Mars2GribDeductionException("Duration overflow while converting `" + key + "` from hours", Here());
    }

    {
        long result = hours * 3600L;
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

///
/// @brief Parse one month-like enumeration token into its canonical integer value.
///
/// Accepted textual aliases are the month abbreviations and full English month
/// names defined by the MARS keyword specification. Numeric strings are parsed
/// as decimal integers and returned without range validation so that the caller
/// can own the final keyword-contract check.
///
/// Accepted textual aliases are:
/// - `jan`, `january`;
/// - `feb`, `february`;
/// - `mar`, `march`;
/// - `apr`, `april`;
/// - `may`;
/// - `jun`, `june`;
/// - `jul`, `july`;
/// - `aug`, `august`;
/// - `sep`, `september`;
/// - `oct`, `october`;
/// - `nov`, `november`;
/// - `dec`, `december`.
///
/// @param[in] value Source token.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Canonical integer month value.
/// @throws Mars2GribDeductionException if the token is neither a supported
///         month alias nor a valid decimal integer.
///
template <class Cntx_t>
inline long parseMonthEnum(const std::string& value, const std::string& key, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    const std::string normalized = lower(value, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

    if (normalized == "jan" || normalized == "january") {
        {
            long result = 1;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "feb" || normalized == "february") {
        {
            long result = 2;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "mar" || normalized == "march") {
        {
            long result = 3;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "apr" || normalized == "april") {
        {
            long result = 4;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "may") {
        {
            long result = 5;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "jun" || normalized == "june") {
        {
            long result = 6;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "jul" || normalized == "july") {
        {
            long result = 7;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "aug" || normalized == "august") {
        {
            long result = 8;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "sep" || normalized == "september") {
        {
            long result = 9;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "oct" || normalized == "october") {
        {
            long result = 10;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "nov" || normalized == "november") {
        {
            long result = 11;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (normalized == "dec" || normalized == "december") {
        {
            long result = 12;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    {
        long result = parseLongStrict(normalized, key, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
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
template <class Cntx_t>
inline long parseDurationStringSeconds(const std::string& raw, const std::string& key, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    if (raw.empty()) {
        throw Mars2GribDeductionException("Empty duration for `" + key + "`", Here());
    }

    std::string value = lower(raw, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
    char unit         = 'h';
    if (std::isalpha(static_cast<unsigned char>(value.back()))) {
        unit = value.back();
        value.pop_back();
    }

    const long count = parseLongStrict(value, key, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
    switch (unit) {
        case 'h':
            {
                long result = checkedHoursToSeconds(count, key, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 'm':
            if (count > std::numeric_limits<long>::max() / 60L || count < std::numeric_limits<long>::min() / 60L) {
                throw Mars2GribDeductionException("Duration overflow in `" + key + "`", Here());
            }
            {
                long result = count * 60L;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 's':
            {
                long result = count;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 'd':
            if (count > std::numeric_limits<long>::max() / 86400L ||
                count < std::numeric_limits<long>::min() / 86400L) {
                throw Mars2GribDeductionException("Duration overflow in `" + key + "`", Here());
            }
            {
                long result = count * 86400L;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribDeductionException("Unsupported duration unit in `" + key + "`: '" + raw + "'", Here());
    }
}

}  // namespace metkit::mars2grib::backend::deductions::detail

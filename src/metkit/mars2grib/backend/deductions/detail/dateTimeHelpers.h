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
/// @file dateTimeHelpers.h
/// @brief Shared date/time parsing helpers for temporal deductions.
///
/// This header contains the local date and time parsing helpers shared by the
/// temporal datetime deductions. The functions here own only lexical parsing and
/// local value validation; they do not own deduction-level dictionary access or
/// temporal-source composition.
///

#pragma once

#include <algorithm>
#include <array>
#include <string>

#include "eckit/types/Date.h"
#include "eckit/types/Time.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"

namespace metkit::mars2grib::backend::deductions::detail {

inline eckit::Date parseDateLong(long value, const std::string& key) {
    const long year = value / 10000L;
    const long month = (value / 100L) % 100L;
    const long day = value % 100L;

    try {
        return eckit::Date(year, month, day);
    } catch (...) {
        throw Mars2GribDeductionException("Invalid date in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }
}

///
/// @brief Parse a numeric date encoded as `YYYYMMDD`.
///
/// The year, month, and day fields are extracted arithmetically and validated
/// by the `eckit::Date` constructor.
///
/// @param[in] value Numeric date value.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Validated calendar date.
/// @throws Mars2GribDeductionException if the extracted fields do not form a
///         valid `eckit::Date`.
///
inline eckit::Date parseDateString(std::string value, const std::string& key) {
    value = digitsOnly(std::move(value), '-');
    if (value.size() != 8 ||
        !std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw Mars2GribDeductionException("Invalid date syntax in `" + key + "`", Here());
    }

    return parseDateLong(parseLongStrict(value, key), key);
}

///
/// @brief Parse a compact or hyphenated date string.
///
/// Accepted forms are `YYYYMMDD` and `YYYY-MM-DD`.
///
/// @param[in] value Date token.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Validated calendar date.
/// @throws Mars2GribDeductionException for invalid syntax or invalid calendar
///         values.
///
inline eckit::Time parseTimeLong(long value, const std::string& key) {
    if (value < 0) {
        throw Mars2GribDeductionException("Negative time in `" + key + "`", Here());
    }

    long hour = 0;
    long minute = 0;
    long second = 0;

    if (value <= 2359L && value % 100L < 60L) {
        hour = value / 100L;
        minute = value % 100L;
    } else {
        hour = value / 10000L;
        minute = (value / 100L) % 100L;
        second = value % 100L;
    }

    try {
        return eckit::Time(hour, minute, second);
    } catch (...) {
        throw Mars2GribDeductionException("Invalid time in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }
}

///
/// @brief Parse a numeric MARS time in `HHMM` or `HHMMSS` form.
///
/// Values compatible with `HHMM` are interpreted as hour/minute pairs; all
/// other non-negative numeric values are interpreted as `HHMMSS`.
///
/// @param[in] value Numeric time representation.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Validated time of day.
/// @throws Mars2GribDeductionException for negative or invalid time values.
///
inline eckit::Time parseTimeString(std::string value, const std::string& key) {
    if (value.find(':') != std::string::npos) {
        std::array<long, 3> parts{0, 0, 0};
        std::size_t part = 0;
        std::size_t start = 0;

        while (start <= value.size() && part < parts.size()) {
            const std::size_t end = value.find(':', start);
            const std::string token = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
            parts[part++] = parseLongStrict(token, key);
            start = end == std::string::npos ? value.size() + 1 : end + 1;
        }

        if (start <= value.size() || part < 2) {
            throw Mars2GribDeductionException("Invalid colon-separated time in `" + key + "`", Here());
        }

        try {
            return eckit::Time(parts[0], parts[1], parts[2]);
        } catch (...) {
            throw Mars2GribDeductionException("Invalid time in `" + key + "`: '" + value + "'", Here());
        }
    }

    if (!std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw Mars2GribDeductionException("Invalid time syntax in `" + key + "`", Here());
    }

    return parseTimeLong(parseLongStrict(value, key), key);
}

///
/// @brief Parse a textual time in colon-separated or compact numeric form.
///
/// Accepted colon-separated forms are `HH:MM` and `HH:MM:SS`. Without colons,
/// the token must be decimal-only and is delegated to `parseTimeLong()`.
///
/// @param[in] value Time token.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Validated time of day.
/// @throws Mars2GribDeductionException for malformed syntax or invalid time
///         values.
///

}  // namespace metkit::mars2grib::backend::deductions::detail

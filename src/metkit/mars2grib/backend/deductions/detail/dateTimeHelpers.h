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
inline eckit::Date parseDateLong(long value, const std::string& key) {
    const long year  = value / 10000L;
    const long month = (value / 100L) % 100L;
    const long day   = value % 100L;

    try {
        return eckit::Date(year, month, day);
    }
    catch (...) {
        throw Mars2GribDeductionException("Invalid date in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }
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
inline eckit::Date parseDateString(std::string value, const std::string& key) {
    value = digitsOnly(std::move(value), '-');
    if (value.size() != 8 ||
        !std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw Mars2GribDeductionException("Invalid date syntax in `" + key + "`", Here());
    }

    return parseDateLong(parseLongStrict(value, key), key);
}

///
/// @brief Parse a numeric MARS time in `HHMM` form.
///
/// Integer transport cannot preserve leading zeroes. Therefore values such as
/// `12` are interpreted as the compact MARS time `0012`, which corresponds to
/// `00:12:00`.
///
/// Only `HHMM` semantics are accepted. Values greater than `2359` or values
/// whose final two digits do not form a valid minute field are rejected.
/// Seconds are never accepted by this parser.
///
/// @param[in] value Numeric time representation.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Validated time of day.
/// @throws Mars2GribDeductionException for negative or invalid time values.
///
inline eckit::Time parseTimeLong(long value, const std::string& key) {
    if (value < 0) {
        throw Mars2GribDeductionException("Negative time in `" + key + "`", Here());
    }

    if (value > 2359L || value % 100L >= 60L) {
        throw Mars2GribDeductionException("Invalid time in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }

    const long hour   = value / 100L;
    const long minute = value % 100L;

    try {
        return eckit::Time(hour, minute, 0);
    }
    catch (...) {
        throw Mars2GribDeductionException("Invalid time in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }
}

///
/// @brief Parse a textual MARS time in compact or colon-separated form.
///
/// Accepted textual forms are `HHMM` and `HH:MM`. Compact textual times must
/// contain exactly four decimal digits. Colon-separated times must contain
/// exactly two decimal fields and exactly one colon separator. Seconds are never
/// accepted by this parser.
///
/// @param[in] value Time token.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return Validated time of day.
/// @throws Mars2GribDeductionException for malformed syntax or invalid time
///         values.
///
inline eckit::Time parseTimeString(std::string value, const std::string& key) {
    if (value.find(':') != std::string::npos) {
        const std::size_t separator = value.find(':');
        const bool hasExactlyOneColon =
            separator != std::string::npos && value.find(':', separator + 1) == std::string::npos;
        const bool hasExpectedLength = value.size() == 5;

        if (!hasExactlyOneColon || !hasExpectedLength) {
            throw Mars2GribDeductionException("Invalid colon-separated time in `" + key + "`", Here());
        }

        const std::string hourToken   = value.substr(0, separator);
        const std::string minuteToken = value.substr(separator + 1);
        const bool tokensAreTwoDigits = hourToken.size() == 2 && minuteToken.size() == 2;

        if (!tokensAreTwoDigits) {
            throw Mars2GribDeductionException("Invalid colon-separated time in `" + key + "`", Here());
        }

        try {
            return eckit::Time(parseLongStrict(hourToken, key), parseLongStrict(minuteToken, key), 0);
        }
        catch (...) {
            throw Mars2GribDeductionException("Invalid time in `" + key + "`: '" + value + "'", Here());
        }
    }

    const bool hasExactlyFourDigits =
        value.size() == 4 && std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); });
    if (!hasExactlyFourDigits) {
        throw Mars2GribDeductionException("Invalid time syntax in `" + key + "`", Here());
    }

    return parseTimeLong(parseLongStrict(value, key), key);
}

///
}  // namespace metkit::mars2grib::backend::deductions::detail

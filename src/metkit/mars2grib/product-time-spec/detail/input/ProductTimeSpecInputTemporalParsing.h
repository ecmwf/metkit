/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file ProductTimeSpecInputTemporalParsing.h
 * @brief Temporal lexical parsing helpers for ProductTimeSpecInput.
 *
 * This header owns the syntax-level parsing of temporal scalar values consumed
 * by `ProductTimeSpecInput`.
 *
 * It converts textual or numeric source representations into typed temporal
 * values:
 *
 * - elapsed durations normalized to seconds;
 * - calendar dates as `eckit::Date`;
 * - times of day as `eckit::Time`.
 *
 * These helpers perform lexical parsing and local representation checks only.
 * They do not classify the product or enforce later stage-specific semantic
 * rules such as whether a parsed step is allowed for a given product shape.
 *
 * All failures are reported as `Mars2GribGenericException` with the offending
 * key name embedded in the diagnostic.
 */

#pragma once

#include <array>
#include <string>

#include "eckit/types/Date.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputCommon.h"

namespace metkit::mars2grib::product_time_spec::input_detail {

/**
 * @brief Parses a compact duration string and normalizes it to seconds.
 *
 * Accepted suffixes are:
 *
 * - `h`: hours;
 * - `m`: minutes;
 * - `s`: seconds;
 * - `d`: fixed 86400-second days.
 *
 * The suffix is case-insensitive. When omitted, hours are assumed. This helper
 * performs lexical parsing and checked unit conversion only; callers apply
 * semantic restrictions such as non-negativity and whole-hour alignment.
 *
 * @param raw Duration text such as `12`, `12h`, `30m`, `15s`, or `2d`.
 * @param key Source-key name included in diagnostics.
 * @return Duration in seconds.
 * @throws Mars2GribGenericException for empty input, invalid integer syntax,
 *         unsupported units, or arithmetic overflow.
 */
inline long parseDurationStringSeconds(const std::string& raw, const std::string& key) {
    if (raw.empty()) {
        throw Mars2GribGenericException("Empty duration for `" + key + "`", Here());
    }

    std::string value = lower(raw);
    char unit = 'h';
    if (std::isalpha(static_cast<unsigned char>(value.back()))) {
        unit = value.back();
        value.pop_back();
    }
    const long count = parseLongStrict(value, key);

    switch (unit) {
        case 'h': return checkedHoursToSeconds(count, key);
        case 'm':
            if (count > std::numeric_limits<long>::max() / 60L ||
                count < std::numeric_limits<long>::min() / 60L) {
                throw Mars2GribGenericException("Duration overflow in `" + key + "`", Here());
            }
            return count * 60L;
        case 's': return count;
        case 'd':
            if (count > std::numeric_limits<long>::max() / 86400L ||
                count < std::numeric_limits<long>::min() / 86400L) {
                throw Mars2GribGenericException("Duration overflow in `" + key + "`", Here());
            }
            return count * 86400L;
        default:
            throw Mars2GribGenericException("Unsupported duration unit in `" + key + "`: '" + raw + "'", Here());
    }
}

/**
 * @brief Parses an integer date encoded as `YYYYMMDD`.
 *
 * The year, month, and day fields are extracted arithmetically and validated by
 * the `eckit::Date` constructor.
 *
 * @param value Numeric date.
 * @param key   Source-key name included in diagnostics.
 * @return Validated calendar date.
 * @throws Mars2GribGenericException if the extracted fields do not form a valid
 *         `eckit::Date`.
 */
inline eckit::Date parseDateLong(long value, const std::string& key) {
    const long year = value / 10000L;
    const long month = (value / 100L) % 100L;
    const long day = value % 100L;
    try {
        return eckit::Date(year, month, day);
    } catch (...) {
        throw Mars2GribGenericException("Invalid date in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }
}

/**
 * @brief Parses a compact or hyphenated date string.
 *
 * Both `YYYYMMDD` and `YYYY-MM-DD` are accepted. Hyphens are removed, after
 * which exactly eight decimal digits are required. Calendar validity is checked
 * by `parseDateLong()`.
 *
 * @param value Date text.
 * @param key   Source-key name included in diagnostics.
 * @return Validated calendar date.
 * @throws Mars2GribGenericException for invalid syntax or an invalid date.
 */
inline eckit::Date parseDateString(std::string value, const std::string& key) {
    value = digitsOnly(std::move(value), '-');
    if (value.size() != 8 ||
        !std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw Mars2GribGenericException("Invalid date syntax in `" + key + "`", Here());
    }
    return parseDateLong(parseLongStrict(value, key), key);
}

/**
 * @brief Parses numeric MARS time in `HHMM` or normalized `HHMMSS` form.
 *
 * Non-negative values not exceeding 2359 whose final two digits form a valid
 * minute field are interpreted as `HHMM`; other non-negative values are
 * interpreted as `HHMMSS`. Final range validation is delegated to
 * `eckit::Time`.
 *
 * Examples:
 *
 * - `0` -> 00:00:00;
 * - `30` -> 00:30:00;
 * - `1230` -> 12:30:00;
 * - `123045` -> 12:30:45.
 *
 * @param value Numeric time representation.
 * @param key   Source-key name included in diagnostics.
 * @return Validated time of day.
 * @throws Mars2GribGenericException for negative or invalid time values.
 */
inline eckit::Time parseTimeLong(long value, const std::string& key) {
    if (value < 0) {
        throw Mars2GribGenericException("Negative time in `" + key + "`", Here());
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
        throw Mars2GribGenericException("Invalid time in `" + key + "`: '" + std::to_string(value) + "'", Here());
    }
}

/**
 * @brief Parses textual time in colon-separated or compact numeric form.
 *
 * Accepted colon forms are `HH:MM` and `HH:MM:SS`. Each component is parsed as
 * a complete integer. Without colons, every character must be decimal and the
 * value is delegated to `parseTimeLong()`.
 *
 * @param value Time text.
 * @param key   Source-key name included in diagnostics.
 * @return Validated time of day.
 * @throws Mars2GribGenericException for malformed component counts, invalid
 *         integer syntax, non-digit compact syntax, or invalid time values.
 */
inline eckit::Time parseTimeString(std::string value, const std::string& key) {
    if (value.find(':') != std::string::npos) {
        std::array<long, 3> parts{0, 0, 0};
        std::size_t part = 0;
        std::size_t start = 0;
        while (start <= value.size() && part < parts.size()) {
            const std::size_t end = value.find(':', start);
            const std::string token = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
            parts[part++] = parseLongStrict(token, key);
            if (end == std::string::npos) {
                start = value.size() + 1;
            } else {
                start = end + 1;
            }
        }
        if (start <= value.size() || part < 2) {
            throw Mars2GribGenericException("Invalid colon-separated time in `" + key + "`", Here());
        }
        try {
            return eckit::Time(parts[0], parts[1], parts[2]);
        } catch (...) {
            throw Mars2GribGenericException("Invalid time in `" + key + "`: '" + value + "'", Here());
        }
    }

    if (!std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw Mars2GribGenericException("Invalid time syntax in `" + key + "`", Here());
    }
    return parseTimeLong(parseLongStrict(value, key), key);
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail

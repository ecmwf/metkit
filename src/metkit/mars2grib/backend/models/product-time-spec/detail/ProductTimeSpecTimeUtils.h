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
/// @file ProductTimeSpecTimeUtils.h
/// @brief Shared time arithmetic helpers for ProductTimeSpec model details.
///

#pragma once

#include <limits>
#include <stdexcept>

#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"
#include "metkit/mars2grib/backend/deductions/common.h"

namespace metkit::mars2grib::backend::models::detail {

inline constexpr long productTimeSpecSecondsPerHour = 3600L;
inline constexpr long productTimeSpecSecondsPerDay = 24L * productTimeSpecSecondsPerHour;
inline const eckit::Time productTimeSpecDefaultTime{0, 0, 0};

///
/// @brief Multiply two non-negative duration factors with overflow checking.
///
/// The ProductTimeSpec model uses this helper when converting hour-based
/// durations to elapsed seconds. The intended callers pass non-negative values,
/// so the overflow logic is specialized for that domain.
///
/// @param[in] lhs Left multiplication factor.
/// @param[in] rhs Right multiplication factor.
/// @param[in] context Human-readable operation label for diagnostics.
/// @return The exact product `lhs * rhs`, or zero when either factor is zero.
/// @throws std::overflow_error if the product cannot be represented by `long`.
///
inline long checkedMultiplyProductTimeSpecTime_or_throw(long lhs,
                                                        long rhs,
                                                        const char* context) {
    if (lhs == 0 || rhs == 0) {
        return 0;
    }
    if (lhs > 0 && rhs > 0 && lhs > std::numeric_limits<long>::max() / rhs) {
        throw std::overflow_error(std::string(context) + ": duration overflow");
    }
    return lhs * rhs;
}

///
/// @brief Test whether a datetime is aligned to midnight.
///
/// @param[in] value Datetime to inspect.
/// @return `true` when hour, minute, and second are all zero.
///
inline bool isAtMidnight(const eckit::DateTime& value) {
    return value.time().hours() == 0 && value.time().minutes() == 0 &&
           value.time().seconds() == 0;
}

///
/// @brief Test whether a datetime is on day one of a month at midnight.
///
/// @param[in] value Datetime to inspect.
/// @return `true` when the datetime is month-aligned for calendar-month arithmetic.
///
inline bool isOnFirstOfMonthMidnight(const eckit::DateTime& value) {
    return isAtMidnight(value) && value.date().day() == 1;
}

///
/// @brief Add a non-negative elapsed number of seconds to a datetime.
///
/// @param[in] value Starting datetime.
/// @param[in] seconds Non-negative elapsed seconds.
/// @return The datetime shifted forward by `seconds`.
/// @throws std::invalid_argument if `seconds` is negative.
///
inline eckit::DateTime addSecondsProductTimeSpecTime_or_throw(const eckit::DateTime& value,
                                                              long seconds) {
    if (seconds < 0) {
        throw std::invalid_argument("elapsed-second addition requires a non-negative duration");
    }
    return value + static_cast<eckit::Second>(seconds);
}

///
/// @brief Subtract a non-negative elapsed number of seconds from a datetime.
///
/// The implementation performs explicit borrow-aware subtraction on the date
/// and second-of-day components because the required eckit operation is not
/// expressed here as direct negative-second addition.
///
/// @param[in] value Starting datetime.
/// @param[in] seconds Non-negative elapsed seconds.
/// @return The datetime shifted backward by `seconds`.
/// @throws std::invalid_argument if `seconds` is negative.
///
inline eckit::DateTime subtractSecondsProductTimeSpecTime_or_throw(const eckit::DateTime& value,
                                                                   long seconds) {
    if (seconds < 0) {
        throw std::invalid_argument("elapsed-second subtraction requires a non-negative duration");
    }

    eckit::Date date = value.date();
    eckit::Second time = value.time();

    const long wholeDays = seconds / productTimeSpecSecondsPerDay;
    const long remainingSeconds = seconds % productTimeSpecSecondsPerDay;
    date -= wholeDays;

    if (time < static_cast<eckit::Second>(remainingSeconds)) {
        date -= 1;
        time += static_cast<eckit::Second>(productTimeSpecSecondsPerDay);
    }

    time -= static_cast<eckit::Second>(remainingSeconds);
    return eckit::DateTime(date, eckit::Time(time));
}

///
/// @brief Shift a month-aligned datetime by a signed number of calendar months.
///
/// Calendar-month arithmetic is defined only for datetimes on day one at
/// midnight. The helper linearizes `(year, month)` into one month index,
/// applies the signed shift, and rebuilds the result on day one at midnight.
///
/// @param[in] value Month-aligned starting datetime.
/// @param[in] months Signed month displacement.
/// @return The shifted first-of-month datetime at midnight.
/// @throws std::invalid_argument if `value` is not month-aligned.
///
inline eckit::DateTime shiftCalendarMonthsProductTimeSpecTime_or_throw(const eckit::DateTime& value,
                                                                       long months) {
    if (!isOnFirstOfMonthMidnight(value)) {
        throw std::invalid_argument(
            "calendar-month arithmetic requires day=1 at 00:00:00");
    }

    const long year = value.date().year();
    const long monthIndex = value.date().month() - 1L;
    const long total = year * 12L + monthIndex + months;

    long newYear = total / 12L;
    long newMonthIndex = total % 12L;
    if (newMonthIndex < 0) {
        newMonthIndex += 12L;
        --newYear;
    }

    return eckit::DateTime(eckit::Date(newYear, newMonthIndex + 1L, 1L),
                           productTimeSpecDefaultTime);
}

///
/// @brief Add one normalized model duration to a datetime.
///
/// Supported units are:
/// - `Second`: elapsed-second arithmetic;
/// - `Hour`: elapsed-time arithmetic after checked hour-to-second conversion;
/// - `Day`: calendar-day arithmetic, requiring a midnight datetime;
/// - `Month`: calendar-month arithmetic, requiring day one at midnight.
///
/// @param[in] value Starting datetime.
/// @param[in] duration Normalized duration to add.
/// @return The resulting datetime.
/// @throws std::invalid_argument on unsupported units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if the hour-to-second conversion overflows.
///
inline eckit::DateTime addProductTimeSpecDuration_or_throw(const eckit::DateTime& value,
                                                           const deductions::TimeDuration& duration) {
    if (duration.length < 0) {
        throw std::invalid_argument("duration length must be non-negative");
    }

    switch (duration.unit) {
        case tables::TimeUnit::Second:
            return addSecondsProductTimeSpecTime_or_throw(value, duration.length);
        case tables::TimeUnit::Hour:
            return addSecondsProductTimeSpecTime_or_throw(
                value,
                checkedMultiplyProductTimeSpecTime_or_throw(
                    duration.length,
                    productTimeSpecSecondsPerHour,
                    "hour duration"));
        case tables::TimeUnit::Day: {
            if (!isAtMidnight(value)) {
                throw std::invalid_argument(
                    "calendar-day arithmetic requires a midnight datetime");
            }
            eckit::Date date = value.date();
            date += duration.length;
            return eckit::DateTime(date, productTimeSpecDefaultTime);
        }
        case tables::TimeUnit::Month:
            return shiftCalendarMonthsProductTimeSpecTime_or_throw(value, duration.length);
        default:
            throw std::invalid_argument("unsupported ProductTimeSpec duration unit");
    }
}

///
/// @brief Subtract one normalized model duration from a datetime.
///
/// Supported units and semantics mirror `addProductTimeSpecDuration_or_throw`.
///
/// @param[in] value Starting datetime.
/// @param[in] duration Normalized duration to subtract.
/// @return The resulting datetime.
/// @throws std::invalid_argument on unsupported units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if the hour-to-second conversion overflows.
///
inline eckit::DateTime subtractProductTimeSpecDuration_or_throw(const eckit::DateTime& value,
                                                                const deductions::TimeDuration& duration) {
    if (duration.length < 0) {
        throw std::invalid_argument("duration length must be non-negative");
    }

    switch (duration.unit) {
        case tables::TimeUnit::Second:
            return subtractSecondsProductTimeSpecTime_or_throw(value, duration.length);
        case tables::TimeUnit::Hour:
            return subtractSecondsProductTimeSpecTime_or_throw(
                value,
                checkedMultiplyProductTimeSpecTime_or_throw(
                    duration.length,
                    productTimeSpecSecondsPerHour,
                    "hour duration"));
        case tables::TimeUnit::Day: {
            if (!isAtMidnight(value)) {
                throw std::invalid_argument(
                    "calendar-day arithmetic requires a midnight datetime");
            }
            eckit::Date date = value.date();
            date -= duration.length;
            return eckit::DateTime(date, productTimeSpecDefaultTime);
        }
        case tables::TimeUnit::Month:
            return shiftCalendarMonthsProductTimeSpecTime_or_throw(value, -duration.length);
        default:
            throw std::invalid_argument("unsupported ProductTimeSpec duration unit");
    }
}

}  // namespace metkit::mars2grib::backend::models::detail

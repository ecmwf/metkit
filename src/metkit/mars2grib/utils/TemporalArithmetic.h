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
/// @file TemporalArithmetic.h
/// @brief Adapt eckit date/time arithmetic to mars2grib temporal semantics.
///
/// These low-level helpers do not receive backend-model input snapshots;
/// therefore their exception frames use the location-only constructor.
/// Higher-level callers may subsequently add richer context.
///
/// Calendar-month arithmetic is never approximated with a fixed number of
/// seconds. Calendar operations are resolved explicitly from `eckit::Date`
/// components to preserve month semantics.
///
/// @ingroup mars2grib_utils
///
#pragma once


#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <optional>

#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib::utils::time_arithmetic {

namespace detail {

///
/// @brief Convert an `eckit::Second` to whole seconds, rejecting sub-second input.
///
/// `eckit::Second` is a `double`; temporal arithmetic here operates on whole
/// seconds only. Any value that is not (within tolerance) integral, or that
/// cannot be represented as a `long`, is rejected.
///
inline long toWholeSeconds(eckit::Second value) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const double rounded = std::nearbyint(value);

        if (std::abs(value - rounded) > 1.0e-6) {
            throw Mars2GribGenericException("Sub-second precision is not supported in temporal arithmetic", Here());
        }

        if (rounded > static_cast<double>(std::numeric_limits<long>::max()) ||
            rounded < static_cast<double>(std::numeric_limits<long>::min())) {
            throw Mars2GribGenericException("Whole-second value is out of range for a long", Here());
        }

        return static_cast<long>(rounded);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Failed to convert an eckit::Second to whole seconds", Here()));
    }
}

///
/// @brief Multiply a duration length by a per-unit second count with overflow checks.
///
/// The multiplication is performed in `long long` and is rejected if it would
/// overflow.
///
inline long long checkedSecondsFromUnits(long length, long long secondsPerUnit) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const long long lengthWide = static_cast<long long>(length);

        if (lengthWide > 0 && lengthWide > std::numeric_limits<long long>::max() / secondsPerUnit) {
            throw Mars2GribGenericException("TimeDuration length overflows when converted to seconds", Here());
        }

        if (lengthWide < 0 && lengthWide < std::numeric_limits<long long>::min() / secondsPerUnit) {
            throw Mars2GribGenericException("TimeDuration length underflows when converted to seconds", Here());
        }

        return lengthWide * secondsPerUnit;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to convert a TimeDuration length to seconds", Here()));
    }
}

inline bool isLeapYear(long year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

inline long daysInMonth(long year, long month) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        switch (month) {
            case 1:
                return 31;
            case 2:
                return isLeapYear(year) ? 29 : 28;
            case 3:
                return 31;
            case 4:
                return 30;
            case 5:
                return 31;
            case 6:
                return 30;
            case 7:
                return 31;
            case 8:
                return 31;
            case 9:
                return 30;
            case 10:
                return 31;
            case 11:
                return 30;
            case 12:
                return 31;
            default:
                throw Mars2GribGenericException("Invalid calendar month in daysInMonth", Here());
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Failed to resolve the number of days in a calendar month", Here()));
    }
}

inline eckit::DateTime shiftDateTimeBySeconds(const eckit::DateTime& dateTime, long long deltaSeconds) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        constexpr long long secondsPerDay = 86400LL;

        const long long currentTimeInSeconds = static_cast<long long>(toWholeSeconds(dateTime.time()));

        if (deltaSeconds > 0 && currentTimeInSeconds > std::numeric_limits<long long>::max() - deltaSeconds) {
            throw Mars2GribGenericException("DateTime second shift overflows", Here());
        }

        if (deltaSeconds < 0 && currentTimeInSeconds < std::numeric_limits<long long>::min() - deltaSeconds) {
            throw Mars2GribGenericException("DateTime second shift underflows", Here());
        }

        const long long totalSeconds = currentTimeInSeconds + deltaSeconds;

        long long dayOffset         = totalSeconds / secondsPerDay;
        long long normalizedSeconds = totalSeconds % secondsPerDay;

        if (normalizedSeconds < 0) {
            normalizedSeconds += secondsPerDay;
            --dayOffset;
        }

        if (dayOffset > static_cast<long long>(std::numeric_limits<long>::max()) ||
            dayOffset < static_cast<long long>(std::numeric_limits<long>::min())) {
            throw Mars2GribGenericException("DateTime day offset is out of range for a long", Here());
        }

        eckit::Date shiftedDate = dateTime.date();
        shiftedDate += static_cast<long>(dayOffset);

        return eckit::DateTime{shiftedDate, eckit::Time{static_cast<long>(normalizedSeconds)}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to shift DateTime by elapsed seconds", Here()));
    }
}

inline eckit::DateTime shiftCalendarMonths(const eckit::DateTime& dateTime, long deltaMonths) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        constexpr long monthShiftBound = 12000000L;

        if (std::abs(deltaMonths) > monthShiftBound) {
            throw Mars2GribGenericException("Calendar-month shift magnitude is out of the supported range", Here());
        }

        const eckit::Date& date       = dateTime.date();
        const auto absoluteMonthIndex = static_cast<long long>(date.year() - 1) * 12LL +
                                        static_cast<long long>(date.month() - 1) + static_cast<long long>(deltaMonths);

        auto yearIndex  = absoluteMonthIndex / 12LL;
        auto monthIndex = absoluteMonthIndex % 12LL;

        if (monthIndex < 0) {
            monthIndex += 12LL;
            --yearIndex;
        }

        const long targetYear  = static_cast<long>(yearIndex + 1);
        const long targetMonth = static_cast<long>(monthIndex + 1);

        if (targetYear < 100) {
            throw Mars2GribGenericException(
                "Calendar-month shift produced a year outside the eckit-safe range (>= 100)", Here());
        }

        const long targetDay = std::min(date.day(), daysInMonth(targetYear, targetMonth));

        return eckit::DateTime{eckit::Date{targetYear, targetMonth, targetDay}, dateTime.time()};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to shift DateTime by calendar months", Here()));
    }
}

}  // namespace detail

///
/// @brief Return the canonical default MARS time.
///
/// @return `00:00:00` as an `eckit::Time`.
/// @throws Mars2GribGenericException If construction fails unexpectedly.
///
inline eckit::Time defaultMarsTime() {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        return eckit::Time{0};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to construct the default MARS time", Here()));
    }
}

///
/// @brief Construct a DateTime from an explicit date and time.
///
/// @param[in] date Calendar date.
/// @param[in] time Time of day.
/// @return Combined eckit DateTime.
/// @throws Mars2GribGenericException If eckit construction fails.
///
inline eckit::DateTime makeDateTime(const eckit::Date& date, const eckit::Time& time) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        return eckit::DateTime{date, time};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Failed to construct DateTime from explicit date and time", Here()));
    }
}

///
/// @brief Construct a DateTime from a date and optional time.
///
/// Missing time is replaced by the canonical default `00:00:00`.
///
/// @param[in] date Calendar date.
/// @param[in] time Optional time of day.
/// @return Combined eckit DateTime.
/// @throws Mars2GribGenericException If defaulting or construction fails.
///
inline eckit::DateTime makeDateTime(const eckit::Date& date, const std::optional<eckit::Time>& time) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const eckit::Time resolvedTime = time.has_value() ? *time : defaultMarsTime();

        return makeDateTime(date, resolvedTime);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to construct DateTime from optional time", Here()));
    }
}

///
/// @brief Construct the canonical zero-duration value.
///
/// @return ProductTimeDuration represented as zero seconds.
/// @throws Mars2GribGenericException If construction fails unexpectedly.
///
inline metkit::mars2grib::backend::deductions::TimeDuration zeroDuration() {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        return TimeDuration{0, TimeUnit::Second};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to construct a zero ProductTimeDuration", Here()));
    }
}

///
/// @brief Return the canonical one-calendar-month duration.
///
/// @return Duration `{1, Month}`.
/// @throws Mars2GribGenericException If construction of the duration unexpectedly fails.
///
inline metkit::mars2grib::backend::deductions::TimeDuration oneMonth() {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        return {1, TimeUnit::Month};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to execute `oneMonth`", Here()));
    }
}

///
/// @brief Return the intrinsic twenty-four-hour synoptic increment.
///
/// @return Duration `{24, Hour}`.
/// @throws Mars2GribGenericException If construction of the duration unexpectedly fails.
///
inline metkit::mars2grib::backend::deductions::TimeDuration twentyFourHours() {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        return {24, TimeUnit::Hour};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to execute `twentyFourHours`", Here()));
    }
}


inline long convertToSeconds(const metkit::mars2grib::backend::deductions::TimeDuration& duration) {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        long long seconds = 0;

        switch (duration.unit) {
            case TimeUnit::Second:
                return duration.length;

            case TimeUnit::Hour:
                seconds = detail::checkedSecondsFromUnits(duration.length, 3600LL);
                break;

            case TimeUnit::Day:
                seconds = detail::checkedSecondsFromUnits(duration.length, 86400LL);
                break;

            case TimeUnit::Month:
                throw Mars2GribGenericException("Cannot convert a calendar-month duration to seconds", Here());

            default:
                throw Mars2GribGenericException("Unsupported TimeDuration unit in convertToSeconds", Here());
        }

        if (seconds > static_cast<long long>(std::numeric_limits<long>::max()) ||
            seconds < static_cast<long long>(std::numeric_limits<long>::min())) {
            throw Mars2GribGenericException("Converted duration in seconds is out of range for a long", Here());
        }

        return static_cast<long>(seconds);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to convert ProductTimeDuration to seconds", Here()));
    }
}

///
/// @brief Add elapsed seconds to a DateTime.
///
/// @param[in] dateTime Base DateTime.
/// @param[in] seconds Elapsed seconds to add.
/// @return Shifted DateTime.
/// @throws Mars2GribGenericException If eckit arithmetic fails.
///
inline eckit::DateTime addSeconds(const eckit::DateTime& dateTime, long seconds) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        return detail::shiftDateTimeBySeconds(dateTime, seconds);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to add seconds to DateTime", Here()));
    }
}

///
/// @brief Subtract elapsed seconds from a DateTime.
///
/// @param[in] dateTime Base DateTime.
/// @param[in] seconds Elapsed seconds to subtract.
/// @return Shifted DateTime.
/// @throws Mars2GribGenericException If eckit arithmetic fails.
///
inline eckit::DateTime subtractSeconds(const eckit::DateTime& dateTime, long seconds) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        if (seconds == std::numeric_limits<long>::min()) {
            throw Mars2GribGenericException("Cannot subtract the minimum representable second count", Here());
        }

        return detail::shiftDateTimeBySeconds(dateTime, -seconds);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to subtract seconds from DateTime", Here()));
    }
}

///
/// @brief Add a TimeDuration to a DateTime.
///
/// Fixed units are converted to seconds. Calendar-month addition preserves the
/// time of day and clamps impossible month-end dates to the last valid day.
///
/// @param[in] dateTime Base DateTime.
/// @param[in] duration Duration to add.
/// @return Shifted DateTime.
/// @throws Mars2GribGenericException For unsupported units or arithmetic failures.
///
inline eckit::DateTime addDuration(const eckit::DateTime& dateTime,
                                   const metkit::mars2grib::backend::deductions::TimeDuration& duration) {

    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        switch (duration.unit) {
            case TimeUnit::Second:
                return addSeconds(dateTime, duration.length);

            case TimeUnit::Hour:
                return detail::shiftDateTimeBySeconds(dateTime,
                                                      detail::checkedSecondsFromUnits(duration.length, 3600LL));

            case TimeUnit::Day:
                return detail::shiftDateTimeBySeconds(dateTime,
                                                      detail::checkedSecondsFromUnits(duration.length, 86400LL));

            case TimeUnit::Month:
                if (duration.length == std::numeric_limits<long>::min()) {
                    throw Mars2GribGenericException("Cannot add the minimum representable month count", Here());
                }

                return detail::shiftCalendarMonths(dateTime, duration.length);

            default:
                throw Mars2GribGenericException("Unsupported TimeDuration unit in addDuration", Here());
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to add TimeDuration to DateTime", Here()));
    }
}

///
/// @brief Subtract a TimeDuration from a DateTime.
///
/// Fixed units are converted to seconds. Calendar-month subtraction preserves
/// the time of day and clamps impossible month-end dates to the last valid day.
///
/// @param[in] dateTime Base DateTime.
/// @param[in] duration Duration to subtract.
/// @return Shifted DateTime.
/// @throws Mars2GribGenericException For unsupported units or arithmetic failures.
///
inline eckit::DateTime subtractDuration(const eckit::DateTime& dateTime,
                                        const metkit::mars2grib::backend::deductions::TimeDuration& duration) {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        switch (duration.unit) {
            case TimeUnit::Second:
                return subtractSeconds(dateTime, duration.length);

            case TimeUnit::Hour: {
                const long long seconds = detail::checkedSecondsFromUnits(duration.length, 3600LL);
                if (seconds == std::numeric_limits<long long>::min()) {
                    throw Mars2GribGenericException("Cannot subtract the minimum representable second count", Here());
                }
                return detail::shiftDateTimeBySeconds(dateTime, -seconds);
            }

            case TimeUnit::Day: {
                const long long seconds = detail::checkedSecondsFromUnits(duration.length, 86400LL);
                if (seconds == std::numeric_limits<long long>::min()) {
                    throw Mars2GribGenericException("Cannot subtract the minimum representable second count", Here());
                }
                return detail::shiftDateTimeBySeconds(dateTime, -seconds);
            }

            case TimeUnit::Month:
                if (duration.length == std::numeric_limits<long>::min()) {
                    throw Mars2GribGenericException("Cannot subtract the minimum representable month count", Here());
                }

                return detail::shiftCalendarMonths(dateTime, -duration.length);

            default:
                throw Mars2GribGenericException("Unsupported TimeDuration unit in subtractDuration", Here());
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to subtract TimeDuration from DateTime", Here()));
    }
}

///
/// @brief Return the first instant of the calendar month following a DateTime.
///
/// This operation is required by synoptic analysis domains and is expressed in
/// calendar units rather than approximated in seconds.
///
/// @param[in] dateTime DateTime whose following month boundary is required.
/// @return First day of the following month at `00:00:00`.
/// @throws Mars2GribGenericException If calendar-month resolution fails.
///
inline eckit::DateTime beginningOfNextCalendarMonth(const eckit::DateTime& dateTime) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const eckit::Date& date = dateTime.date();
        const long year         = date.year();
        const long month        = date.month();

        const bool isDecember = month == 12;
        const long nextYear   = isDecember ? year + 1 : year;
        const long nextMonth  = isDecember ? 1 : month + 1;

        return makeDateTime(eckit::Date{nextYear, nextMonth, 1}, defaultMarsTime());
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to resolve the next calendar-month boundary", Here()));
    }
}

///
/// @brief Compute the non-negative elapsed duration between two DateTimes.
///
/// @param[in] begin Inclusive beginning of the interval.
/// @param[in] end Exclusive end of the interval.
/// @return Elapsed interval represented in seconds.
/// @throws Mars2GribGenericException If `end` precedes `begin` or arithmetic fails.
///
inline metkit::mars2grib::backend::deductions::TimeDuration durationBetween(const eckit::DateTime& begin,
                                                                            const eckit::DateTime& end) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const eckit::Second elapsedSeconds = end - begin;
        const bool durationIsNonNegative   = end >= begin;

        if (!durationIsNonNegative) {
            throw Mars2GribGenericException("ProductTimeSpec domain duration cannot be negative", Here());
        }

        const long seconds = detail::toWholeSeconds(elapsedSeconds);

        return TimeDuration{seconds, TimeUnit::Second};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Failed to compute elapsed ProductTimeSpec duration", Here()));
    }
}

}  // namespace metkit::mars2grib::utils::time_arithmetic

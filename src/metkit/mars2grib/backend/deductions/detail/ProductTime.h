/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file detail/ProductTime.h
/// @brief Implementation detail of the `ProductTime` deduction.
///
/// Defines:
///   - `TimespanKind`              (§6)
///   - `ProductTimeInput`          (§6)
///   - `ProductTime`               (§5)
///   - `make_ProductTime_or_throw` (§6)
///   - shared helpers (calendar arithmetic, alignment checks, signed-second
///     shifts, fmt overloads)
///
/// Types extracted to dedicated shared headers:
///   - `StatisticalWindow`            → `detail/StatisticalWindow.h` (§21)
///   - `parse_StatType_or_throw`,
///     `ParsedStatTypeBlock`          → `detail/StatType.h`           (§22)
///
/// This is the **detail** half of the `ProductTime` module. The public
/// resolver `resolve_ProductTime_or_throw` lives in
/// `deductions/productTime.h` (function-primary, lowercase initial per
/// §20.1; this file is type-primary and uses UpperCamelCase initial).
///
/// All rules in this file are normative; see `deductions/timeProducts.md`.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

// System includes
#include <array>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

// eckit
#include "eckit/exception/Exceptions.h"
#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

// Project
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/StatisticalWindow.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions::detail {

// =============================================================
// 1. Constants
// =============================================================

///
/// @brief Maximum number of statistical windows a `ProductTime` may carry.
///
/// Matches the maximum producible by the locked `stattype` grammar
/// (§7.7): one `mo` block + one `da` block + one inner `timespan` window.
///
inline constexpr std::size_t maxStatisticalWindows = 3;

// =============================================================
// 2. ProductTime (§5)
// =============================================================

///
/// @brief Canonical, immutable representation of all temporal information
///        associated with a single MARS product.
///
/// All members are `const`; the struct is copyable but not assignable.
///
/// See `deductions/timeProducts.md` §5 for the full contract, including:
///   - tri-equivalent instant invariant (§5.1)
///   - window-end ordering invariant   (§5.2)
///   - reference-vs-initial-conditions invariant (§5.3)
///   - per-consumer field-access table  (§15)
///
struct ProductTime {

    const eckit::DateTime labelDateTime;
    const eckit::DateTime initialConditionsDateTime;
    const eckit::DateTime referenceDateTime;

    /// Internal convention: `[windowStart, windowEnd)` when
    /// `windowStart < windowEnd`. For instant products
    /// `windowStart == windowEnd` (the point in time at `windowEnd`).
    const eckit::DateTime windowStart;
    const eckit::DateTime windowEnd;

    /// Valid entries: `statisticalWindows[0 .. statisticalWindowCount)`.
    /// Ordering: outermost → innermost.
    const std::array<StatisticalWindow, maxStatisticalWindows> statisticalWindows;
    const std::size_t statisticalWindowCount;

    /// Sampling increment of the innermost statistical loop.
    /// `std::nullopt` for instant products (§9.1).
    /// Optionally absent for single-window statistical products (AIFS path,
    /// §9.4). Required and `> 0` for multi-window statistical products.
    const std::optional<long> timeIncrementInSeconds;
};

// =============================================================
// 3. TimespanKind (§6)
// =============================================================

///
/// @brief Origin of the MARS `timespan` keyword in a `ProductTimeInput`.
///
enum class TimespanKind {
    Missing,   ///< MARS keyword absent.
    Duration,  ///< MARS keyword carries a duration value.
    None       ///< MARS keyword equals literal `"none"` (fakeDoubleLoop, §9.4).
};

// =============================================================
// 4. ProductTimeInput (§6)
// =============================================================

///
/// @brief Resolver-side input bundle for `make_ProductTime_or_throw`.
///
/// The resolver normalizes raw MARS / par keys into this struct before
/// invoking the factory. The factory does not consume raw MARS keys.
///
struct ProductTimeInput {

    eckit::DateTime labelDateTime;

    std::optional<eckit::DateTime> initialConditionsDateTime;  ///< from `hdate` / `htime`
    std::optional<eckit::DateTime> referenceDateTime;  ///< from `fcyear` / `fcmonth`

    /// Offset from `referenceDateTime` to `ProductTime::windowEnd`.
    long stepInSeconds{0};

    TimespanKind timespanKind{TimespanKind::Missing};

    /// Valid only when `timespanKind == TimespanKind::Duration`.
    StatisticalWindow timespan{};

    /// Temporal windows decoded from `stattype` (period part only),
    /// ordered outermost → innermost.
    std::array<StatisticalWindow, maxStatisticalWindows> stattypeWindows{};
    std::size_t stattypeWindowCount{0};

    /// From `deductions::timeIncrementInSeconds_opt(mars, par)`, i.e.
    /// `par["timeIncrementInSeconds"]`. Absent for instants and for the AIFS
    /// single-window path (§9.4).
    std::optional<long> timeIncrementInSeconds;
};

// =============================================================
// 5. Conversion helpers (§7.5, §7.6)
// =============================================================

///
/// @brief Parse a duration string (`step` / `timespan` syntax) to seconds.
///
/// Bare numeric `N` parses as **N hours**. Suffixed numeric supports:
///   - `h` hours
///   - `m` minutes
///   - `s` seconds
///   - `d` days
///
/// @throws Mars2GribDeductionException on malformed input.
///
inline long toSeconds_or_throw(std::string_view step) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (step.empty()) {
        throw Mars2GribDeductionException("Empty duration string", Here());
    }

    std::size_t pos = 0;
    while (pos < step.size() && std::isdigit(static_cast<unsigned char>(step[pos]))) {
        ++pos;
    }

    if (pos == 0) {
        throw Mars2GribDeductionException("Invalid duration format (no numeric part): '" + std::string(step) + "'",
                                          Here());
    }

    long value = 0;
    try {
        value = std::stol(std::string(step.substr(0, pos)));
    }
    catch (...) {
        throw Mars2GribDeductionException("Invalid numeric value in duration: '" + std::string(step) + "'", Here());
    }

    char unit = 'h';  // default unit
    if (pos < step.size()) {
        if (pos + 1 != step.size()) {
            throw Mars2GribDeductionException(
                "Invalid duration format (trailing characters): '" + std::string(step) + "'", Here());
        }
        unit = step[pos];
    }

    switch (unit) {
        case 'h':
            return value * 3600L;
        case 'm':
            return value * 60L;
        case 's':
            return value;
        case 'd':
            return value * 86400L;
        default:
            throw Mars2GribDeductionException(std::string("Unknown duration unit: '") + unit + "', expected={h,m,s,d}",
                                              Here());
    }
}

///
/// @brief Convert a MARS-encoded `YYYYMMDD` integer to `eckit::Date`.
///
inline eckit::Date convert_YYYYMMDD2Date_or_throw(long YYYYMMDD) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    long YYYY = YYYYMMDD / 10000;
    long MM   = (YYYYMMDD / 100) % 100;
    long DD   = YYYYMMDD % 100;

    try {
        return eckit::Date(YYYY, MM, DD);
    }
    catch (const eckit::Exception& e) {
        throw Mars2GribDeductionException("Invalid date value '" + std::to_string(YYYYMMDD) + "': " + e.what(), Here());
    }
}

///
/// @brief Convert a MARS-encoded `HHMMSS` integer to `eckit::Time`.
///
inline eckit::Time convert_hhmmss2Time_or_throw(long hhmmss) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    long hh = hhmmss / 10000;
    long mm = (hhmmss / 100) % 100;
    long ss = hhmmss % 100;

    try {
        return eckit::Time(hh, mm, ss);
    }
    catch (const eckit::Exception& e) {
        throw Mars2GribDeductionException("Invalid time value '" + std::to_string(hhmmss) + "': " + e.what(), Here());
    }
}

// =============================================================
// 6. Allow-list / classification helpers (§3.1, §3.3)
// =============================================================

///
/// @brief Test whether a `tables::TimeUnit` value belongs to the
///        `StatisticalWindow` allow-list `{Second, Day, Month}` (§3.1).
///
inline bool isAllowedWindowUnit(tables::TimeUnit u) {
    return u == tables::TimeUnit::Second || u == tables::TimeUnit::Day || u == tables::TimeUnit::Month;
}

///
/// @brief Test whether a `tables::TimeUnit` value is calendar-aligned
///        (i.e. `Day` or `Month`) per the classification table in §3.3.
///
inline bool isCalendarUnit(tables::TimeUnit u) {
    return u == tables::TimeUnit::Day || u == tables::TimeUnit::Month;
}

// =============================================================
// 7. Calendar arithmetic (§9.6)
// =============================================================

///
/// @brief Subtract `count` calendar months from a day=1, midnight `DateTime`.
///
/// Precondition: `dt` is on day=1 at 00:00:00 (verified by alignment check
/// (§10.10) before this function is called).
///
inline eckit::DateTime subtractCalendarMonths(const eckit::DateTime& dt, long count) {
    long year  = dt.date().year();
    long month = dt.date().month();

    // Convert to (year * 12 + (month-1)) basis so subtraction is trivial.
    long total       = year * 12L + (month - 1L) - count;
    long newYear     = total / 12L;
    long newMonthIdx = total % 12L;
    if (newMonthIdx < 0) {
        newMonthIdx += 12L;
        newYear -= 1L;
    }
    long newMonth = newMonthIdx + 1L;

    return eckit::DateTime(eckit::Date(newYear, newMonth, 1), eckit::Time(0, 0, 0));
}

///
/// @brief Subtract `count` calendar days from a midnight `DateTime`.
///
/// Precondition: `dt` is at 00:00:00 (verified by alignment check (§10.9)).
///
inline eckit::DateTime subtractCalendarDays(const eckit::DateTime& dt, long count) {
    eckit::Date d = dt.date();
    d -= count;
    return eckit::DateTime(d, eckit::Time(0, 0, 0));
}

///
/// @brief Subtract `count` seconds from a `DateTime`.
///
inline eckit::DateTime subtractSeconds(const eckit::DateTime& dt, long count) {
    return dt + (-static_cast<eckit::Second>(count));
}

///
/// @brief Apply `windowStart := windowEnd - window` per §9.6.
///
/// Dispatches on `window.unit`. Precondition: `window.unit` is in the
/// allow-list and any required alignment has been verified by the caller.
///
inline eckit::DateTime applyWindowSubtraction(const eckit::DateTime& windowEnd, const StatisticalWindow& window) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    switch (window.unit) {
        case tables::TimeUnit::Second:
            return subtractSeconds(windowEnd, window.count);
        case tables::TimeUnit::Day:
            return subtractCalendarDays(windowEnd, window.count);
        case tables::TimeUnit::Month:
            return subtractCalendarMonths(windowEnd, window.count);
        default:
            throw Mars2GribDeductionException("Internal error: applyWindowSubtraction called with disallowed unit",
                                              Here());
    }
    mars2gribUnreachable();
}

// =============================================================
// 8. Alignment checks (§4.2, §4.3, §10.9, §10.10)
// =============================================================

///
/// @brief Test whether a `DateTime` is at hh=00, mm=00, ss=00.
///
inline bool isAtMidnight(const eckit::DateTime& dt) {
    const eckit::Time& t = dt.time();
    return t.hours() == 0 && t.minutes() == 0 && t.seconds() == 0;
}

///
/// @brief Test whether a `DateTime` is on day=1 at midnight.
///
inline bool isOnFirstOfMonthMidnight(const eckit::DateTime& dt) {
    return isAtMidnight(dt) && dt.date().day() == 1;
}

// =============================================================
// 9. Inline string formatting (§12, §13)
// =============================================================

///
/// @brief Format an `eckit::DateTime` as an ISO-8601 string for logs/errors.
///
inline std::string fmt(const eckit::DateTime& dt) {
    return dt.iso(true);
}

///
/// @brief Format a `tables::TimeUnit` value as a short symbolic name.
///
/// Uses the canonical names produced by `tables::enum2name_TimeUnit_or_throw`
/// for the allow-list values, with a numeric fallback for any other value.
///
inline std::string fmt(tables::TimeUnit u) {
    switch (u) {
        case tables::TimeUnit::Second:
            return "second";
        case tables::TimeUnit::Day:
            return "day";
        case tables::TimeUnit::Month:
            return "month";
        default:
            return "TimeUnit(" + std::to_string(static_cast<long>(u)) + ")";
    }
}

///
/// @brief Format a `StatisticalWindow` as `{unit,count}`.
///
inline std::string fmt(const StatisticalWindow& w) {
    return "{" + fmt(w.unit) + "," + std::to_string(w.count) + "}";
}

///
/// @brief Format the populated prefix of a `StatisticalWindow` array.
///
inline std::string fmt(const std::array<StatisticalWindow, maxStatisticalWindows>& a, std::size_t count) {
    std::string s{"["};
    for (std::size_t i = 0; i < count; ++i) {
        if (i)
            s += ",";
        s += fmt(a[i]);
    }
    s += "]";
    return s;
}

// =============================================================
// 10. Factory: make_ProductTime_or_throw (§6, §9, §10)
// =============================================================

///
/// @brief Validate input invariants and construct an immutable `ProductTime`.
///
/// Performs (in order):
///   1. Default resolution for `initialConditionsDateTime` and `referenceDateTime`
///      (§7.3, §7.4).
///   2. Computes `windowEnd := referenceDateTime + stepInSeconds` (§7.5).
///   3. Assembles `statisticalWindows` per the case table (§9).
///   4. Computes `windowStart` per §9.
///   5. Validates all invariants (§5.1, §5.2, §5.3) and all hard errors (§10).
///
/// @throws Mars2GribDeductionException on any rule violation; check sites
///         are tagged with the corresponding §10 entry number.
///
inline ProductTime make_ProductTime_or_throw(const ProductTimeInput& input) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    // ---------------------------------------------------------
    // Default resolution (§7.3, §7.4)
    // ---------------------------------------------------------

    const eckit::DateTime labelDateTime = input.labelDateTime;

    // §7.3: hdate / htime defaulting (the resolver has already enforced
    // §10.2; here we just apply the fall-through to labelDateTime).
    const eckit::DateTime initialConditionsDateTime =
        input.initialConditionsDateTime.value_or(labelDateTime);

    // §7.4: fcyear / fcmonth defaulting.
    const eckit::DateTime referenceDateTime = input.referenceDateTime.value_or(initialConditionsDateTime);

    // ---------------------------------------------------------
    // §5.3: referenceDateTime >= initialConditionsDateTime          (§10.4)
    // ---------------------------------------------------------
    if (referenceDateTime < initialConditionsDateTime) {
        throw Mars2GribDeductionException("ProductTime invariant violated [§10.4]: referenceDateTime ('" +
                                              fmt(referenceDateTime) + "') < initialConditionsDateTime ('" +
                                              fmt(initialConditionsDateTime) + "')",
                                          Here());
    }

    // ---------------------------------------------------------
    // windowEnd (§7.5)
    // ---------------------------------------------------------
    const eckit::DateTime windowEnd = referenceDateTime + static_cast<eckit::Second>(input.stepInSeconds);

    // ---------------------------------------------------------
    // §9: window assembly
    // ---------------------------------------------------------
    std::array<StatisticalWindow, maxStatisticalWindows> windows{};
    std::size_t windowCount     = 0;
    eckit::DateTime windowStart = windowEnd;

    const bool hasTimespanDuration = (input.timespanKind == TimespanKind::Duration);
    const bool hasTimespanNone     = (input.timespanKind == TimespanKind::None);
    const bool hasTimespanMissing  = (input.timespanKind == TimespanKind::Missing);
    const std::size_t nStat        = input.stattypeWindowCount;

    // Case dispatch (§9).
    if (hasTimespanMissing && nStat == 0) {
        // ----- §9.1: Instant product -----
        windowCount = 0;
        windowStart = windowEnd;
    }
    else if (hasTimespanDuration && nStat == 0) {
        // ----- §9.2: Old-style single-loop statistic -----
        windows[0]  = input.timespan;
        windowCount = 1;
        // windowStart computed after allow-list / positivity / alignment.
    }
    else if (hasTimespanDuration && nStat >= 1) {
        // ----- §9.3: Old-style multi-loop statistic -----
        if (nStat + 1 > maxStatisticalWindows) {
            throw Mars2GribDeductionException("ProductTime invariant violated [§10.15]: statisticalWindowCount (" +
                                                  std::to_string(nStat + 1) + ") > maxStatisticalWindows (" +
                                                  std::to_string(maxStatisticalWindows) + ")",
                                              Here());
        }
        for (std::size_t i = 0; i < nStat; ++i) {
            windows[i] = input.stattypeWindows[i];
        }
        windows[nStat] = input.timespan;
        windowCount    = nStat + 1;
    }
    else if (hasTimespanNone && nStat == 1) {
        // ----- §9.4: New-style fakeDoubleLoop -----
        windows[0]  = input.stattypeWindows[0];
        windowCount = 1;
    }
    else if (hasTimespanNone && nStat == 0) {
        // §10.7: timespan = none but stattype missing.
        throw Mars2GribDeductionException(
            "ProductTime invariant violated [§10.7]: timespan='none' requires "
            "exactly one stattype block, got 0",
            Here());
    }
    else if (hasTimespanNone && nStat > 1) {
        // §10.8: timespan = none with more than one stattype block.
        throw Mars2GribDeductionException(
            "ProductTime invariant violated [§10.8]: timespan='none' requires "
            "exactly one stattype block, got " +
                std::to_string(nStat),
            Here());
    }
    else if (hasTimespanMissing && nStat >= 1) {
        // §10.6: stattype present but timespan missing.
        throw Mars2GribDeductionException("ProductTime invariant violated [§10.6]: stattype present (" +
                                              std::to_string(nStat) + " block(s)) but timespan is missing",
                                          Here());
    }
    else {
        // Defensive: all combinations should be covered above.
        throw Mars2GribDeductionException(
            "ProductTime internal error: unhandled (timespanKind, stattypeCount) combination", Here());
    }

    // ---------------------------------------------------------
    // Per-window validation (§3.1, §3.2 → §10.11, §10.18 (b))
    // ---------------------------------------------------------
    for (std::size_t i = 0; i < windowCount; ++i) {
        const StatisticalWindow& w = windows[i];

        if (w.count <= 0) {
            throw Mars2GribDeductionException("ProductTime invariant violated [§10.11]: statisticalWindows[" +
                                                  std::to_string(i) + "] has non-positive count (" +
                                                  std::to_string(w.count) + ")",
                                              Here());
        }

        if (!isAllowedWindowUnit(w.unit)) {
            throw Mars2GribDeductionException("ProductTime invariant violated [§10.18(b)]: statisticalWindows[" +
                                                  std::to_string(i) + "] uses disallowed TimeUnit '" + fmt(w.unit) +
                                                  "', expected one of {second, day, month}",
                                              Here());
        }
    }

    // ---------------------------------------------------------
    // Outermost-window alignment (§4.4, §10.9, §10.10) and windowStart
    // ---------------------------------------------------------
    if (windowCount > 0) {
        const StatisticalWindow& outermost = windows[0];

        if (outermost.unit == tables::TimeUnit::Day) {
            if (!isAtMidnight(windowEnd)) {
                throw Mars2GribDeductionException(
                    "ProductTime invariant violated [§10.9]: outermost window is "
                    "calendar-day-aligned but windowEnd ('" +
                        fmt(windowEnd) + "') is not at hh=00,mm=00,ss=00",
                    Here());
            }
        }
        else if (outermost.unit == tables::TimeUnit::Month) {
            if (!isOnFirstOfMonthMidnight(windowEnd)) {
                throw Mars2GribDeductionException(
                    "ProductTime invariant violated [§10.10]: outermost window is "
                    "calendar-month-aligned but windowEnd ('" +
                        fmt(windowEnd) + "') is not on day=1 at hh=00,mm=00,ss=00",
                    Here());
            }
        }
        // tables::TimeUnit::Second: no alignment required.

        windowStart = applyWindowSubtraction(windowEnd, outermost);
    }

    // ---------------------------------------------------------
    // §5.2: windowStart <= windowEnd                          (defensive)
    // ---------------------------------------------------------
    if (windowStart > windowEnd) {
        throw Mars2GribDeductionException("ProductTime invariant violated [§5.2]: windowStart ('" + fmt(windowStart) +
                                              "') > windowEnd ('" + fmt(windowEnd) + "')",
                                          Here());
    }

    // ---------------------------------------------------------
    // timeIncrementInSeconds validation (§7.8, §9.5, §10.13, §10.14)
    // ---------------------------------------------------------
    std::optional<long> tInc = input.timeIncrementInSeconds;

    if (tInc.has_value() && tInc.value() < 0) {
        throw Mars2GribDeductionException("ProductTime invariant violated [§10.14]: timeIncrementInSeconds < 0 ('" +
                                              std::to_string(tInc.value()) + "')",
                                          Here());
    }

    if (windowCount >= 2 && !tInc.has_value()) {
        throw Mars2GribDeductionException("ProductTime invariant violated [§10.13]: statisticalWindowCount (" +
                                              std::to_string(windowCount) +
                                              ") >= 2 requires timeIncrementInSeconds to be present",
                                          Here());
    }

    // ---------------------------------------------------------
    // §5.1: tri-equivalent instant invariant                  (§10.5)
    // ---------------------------------------------------------
    const bool a = (windowStart == windowEnd);
    const bool b = (windowCount == 0);
    const bool c = !tInc.has_value();
    if (!((a == b) && (b == c))) {
        throw Mars2GribDeductionException(
            std::string("ProductTime invariant violated [§10.5]: tri-equivalence broken: ") +
                "(windowStart==windowEnd)=" + (a ? "true" : "false") + ", (statisticalWindowCount==0)=" +
                (b ? "true" : "false") + ", (timeIncrementInSeconds==nullopt)=" + (c ? "true" : "false"),
            Here());
    }

    // ---------------------------------------------------------
    // Construct the immutable ProductTime
    // ---------------------------------------------------------
    return ProductTime{labelDateTime, initialConditionsDateTime, referenceDateTime, windowStart,
                       windowEnd,          windows,           windowCount,       tInc};
}

}  // namespace metkit::mars2grib::backend::deductions::detail

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
/// @file impl/statisticsDescriptor.h
/// @brief Pure SoA builder for the GRIB Section-4 statistical processing keys.
///
/// Defines:
///   - `StatisticalProcessing`: a transparent shadow of the GRIB header keys
///     `typeOfStatisticalProcessing`, `typeOfTimeIncrement`,
///     `indicatorOfUnitForTimeRange`, `lengthOfTimeRange`,
///     `indicatorOfUnitForTimeIncrement`, `lengthOfTimeIncrement`. All
///     members are `long` / `vector<long>` so the struct can be written
///     verbatim to eccodes.
///
///   - `compute_StatisticalProcessing`: a pure function turning a
///     resolved `ProductTime` plus the per-loop typeOfStatisticalProcessing
///     vector into a fully-populated `StatisticalProcessing`.
///
/// All semantic decisions (instant vs single-window vs multi-window, AIFS
/// no-increment hack, calendar-month length) live inside this single
/// function. The encoding layer remains a thin write-out.
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/ProductTime.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_::impl {

///
/// @brief Transparent SoA shadow of the GRIB Section-4 statistical keys.
///
/// All vectors have the same length: `numberOfTimeRanges`. Ordering is
/// outermost → innermost, matching `ProductTime::statisticalWindows`.
///
/// All members are `long` / `vector<long>` because eccodes does not
/// understand `tables::TimeUnit` or `tables::TypeOfStatisticalProcessing`
/// as types — it expects raw numeric codes. Callers MUST cast the table
/// enums to `static_cast<long>` before populating the vectors (the builder
/// in this file does this internally).
///
struct StatisticalProcessing {
    long numberOfTimeRanges = 0;

    std::vector<long> typeOfStatisticalProcessing;
    std::vector<long> typeOfTimeIncrement;
    std::vector<long> indicatorOfUnitForTimeRange;
    std::vector<long> lengthOfTimeRange;
    std::vector<long> indicatorOfUnitForTimeIncrement;
    std::vector<long> lengthOfTimeIncrement;
};

namespace detail {

///
/// @brief Length in hours of the calendar month that ENDS at `(year, month)`.
///
/// Mirrors the legacy `previousMonthLengthHours` from
/// `deductions/detail/timeUtils.h`. A monthly statistical window whose
/// `windowEnd` is the first of the month covers the *previous* calendar
/// month, hence the helper is named accordingly.
///
inline long previousMonthLengthHours(long year, long month) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (month < 1 || month > 12) {
        throw Mars2GribGenericException("Invalid month (must be 1..12)", Here());
    }

    switch (month) {
        case 1:
        case 2:
        case 4:
        case 6:
        case 8:
        case 9:
        case 11:
            return 31 * 24;

        case 5:
        case 7:
        case 10:
        case 12:
            return 30 * 24;

        case 3:
            return ((year % 4) == 0 ? 29 : 28) * 24;
    }

    throw Mars2GribGenericException("Unreachable", Here());
}

///
/// @brief Convert a `StatisticalWindow` to its length in hours.
///
/// - `Second`: `count` must be a multiple of 3600, returns `count / 3600`.
/// - `Day`:    returns `count * 24`.
/// - `Month`:  returns `count * previousMonthLengthHours(endYear, endMonth)`.
///
/// `endYear`/`endMonth` are taken from `pt.windowEnd` for monthly windows.
///
inline long windowLengthInHours(const deductions::detail::StatisticalWindow& w, long endYear, long endMonth) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    switch (w.unit) {
        case tables::TimeUnit::Second: {
            if (w.count % 3600 != 0) {
                throw Mars2GribDeductionException(
                    "StatisticalWindow with unit=Second must have count divisible by 3600 "
                    "to be expressed in hours; actual count=" +
                        std::to_string(w.count),
                    Here());
            }
            return w.count / 3600;
        }
        case tables::TimeUnit::Day:
            return w.count * 24;
        case tables::TimeUnit::Month:
            return w.count * previousMonthLengthHours(endYear, endMonth);
        default:
            throw Mars2GribDeductionException("Unsupported StatisticalWindow unit for hour conversion: '" +
                                                  tables::enum2name_TimeUnit_or_throw(w.unit) + "'",
                                              Here());
    }
}

}  // namespace detail

///
/// @brief Build the GRIB statistical-processing SoA from a `ProductTime`.
///
/// Single uniform loop over `pt.statisticalWindows[0 .. statisticalWindowCount)`.
/// No instant / single-window / multi-window branching: every slot is
/// filled identically except for the AIFS no-increment edge case detailed
/// below.
///
/// **Per-slot semantics**:
/// - `typeOfTimeIncrement[i]             = 2`         (multIO fixed value)
/// - `indicatorOfUnitForTimeRange[i]     = Hour (1)`  (legacy convention)
/// - `indicatorOfUnitForTimeIncrement[i] = Second (13)` (legacy convention)
/// - `typeOfStatisticalProcessing[i]     = static_cast<long>(types[i])`
/// - `lengthOfTimeRange[i]               = windowLengthInHours(window[i])`
/// - `lengthOfTimeIncrement[i]`:
///     * inner slot (`i == n - 1`): `pt.timeIncrementInSeconds.value()`
///       (or AIFS hack — see below)
///     * outer slot:                `lengthOfTimeRange[i+1] * 3600` (i.e.
///       the hour-length of the next-deeper loop expressed in seconds).
///
/// **AIFS single-window no-increment hack** (§9.4):
/// when `pt.statisticalWindowCount == 1` AND
/// `!pt.timeIncrementInSeconds.has_value()`, the inner slot is filled with
///   - `lengthOfTimeIncrement[0]           = 0`
///   - `indicatorOfUnitForTimeIncrement[0] = static_cast<long>(TimeUnit::Missing)` (255)
/// to mark the increment as undefined.
///
/// **Preconditions**:
/// - `types.size() == pt.statisticalWindowCount`.
/// - For `pt.statisticalWindowCount >= 2`, `pt.timeIncrementInSeconds`
///   MUST have a value (the AIFS hack is single-window only).
///
/// @param[in] pt    Resolved `ProductTime`.
/// @param[in] types Per-loop GRIB type-of-statistical-processing codes
///                  (outermost → innermost), same length as
///                  `pt.statisticalWindowCount`.
///
/// @return Fully-populated `StatisticalProcessing` ready to be written to
///         the GRIB header verbatim.
///
/// @throws Mars2GribDeductionException on size mismatch, missing required
///         increment for multi-window products, or unsupported window unit.
///
inline StatisticalProcessing compute_StatisticalProcessing(
    const deductions::detail::ProductTime& pt, const std::vector<tables::TypeOfStatisticalProcessing>& types) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    const std::size_t n = pt.statisticalWindowCount;

    if (types.size() != n) {
        throw Mars2GribDeductionException(
            "compute_StatisticalProcessing: types.size()=" + std::to_string(types.size()) +
                " != pt.statisticalWindowCount=" + std::to_string(n),
            Here());
    }

    StatisticalProcessing out;
    out.numberOfTimeRanges = static_cast<long>(n);
    out.typeOfStatisticalProcessing.resize(n);
    out.typeOfTimeIncrement.resize(n);
    out.indicatorOfUnitForTimeRange.resize(n);
    out.lengthOfTimeRange.resize(n);
    out.indicatorOfUnitForTimeIncrement.resize(n);
    out.lengthOfTimeIncrement.resize(n);

    if (n == 0) {
        // Instant product: nothing to encode. Caller (Allocate stage) will
        // still write numberOfTimeRanges=0; Preset will write 6 empty
        // vectors which eccodes treats as no-op for these keys.
        return out;
    }

    // AIFS single-window no-increment detection (§9.4).
    const bool aifsSingleWindowHack = (n == 1) && !pt.timeIncrementInSeconds.has_value();

    if (n >= 2 && !pt.timeIncrementInSeconds.has_value()) {
        throw Mars2GribDeductionException("compute_StatisticalProcessing: multi-window product (n=" +
                                              std::to_string(n) + ") requires pt.timeIncrementInSeconds; missing.",
                                          Here());
    }

    const long endYear  = pt.windowEnd.date().year();
    const long endMonth = pt.windowEnd.date().month();

    // First pass: fill typeOfStatisticalProcessing, fixed indicators, and
    // lengthOfTimeRange. lengthOfTimeIncrement is filled in a second pass
    // so outer slots can reference the inner slot's lengthOfTimeRange.
    for (std::size_t i = 0; i < n; ++i) {
        out.typeOfStatisticalProcessing[i]     = static_cast<long>(types[i]);
        out.typeOfTimeIncrement[i]             = 2;
        out.indicatorOfUnitForTimeRange[i]     = static_cast<long>(tables::TimeUnit::Hour);
        out.indicatorOfUnitForTimeIncrement[i] = static_cast<long>(tables::TimeUnit::Second);
        out.lengthOfTimeRange[i] = detail::windowLengthInHours(pt.statisticalWindows[i], endYear, endMonth);
    }

    // Second pass: lengthOfTimeIncrement.
    for (std::size_t i = 0; i < n; ++i) {
        const bool isInner = (i + 1 == n);
        if (isInner) {
            if (aifsSingleWindowHack) {
                out.lengthOfTimeIncrement[i]           = 0;
                out.indicatorOfUnitForTimeIncrement[i] = static_cast<long>(tables::TimeUnit::Missing);
            }
            else {
                // Guaranteed by the multi-window precondition check above
                // (and by single-window non-AIFS path).
                out.lengthOfTimeIncrement[i] = pt.timeIncrementInSeconds.value();
            }
        }
        else {
            // Outer slot: increment equals the next-deeper window length
            // expressed in seconds (lengthOfTimeRange is in hours).
            out.lengthOfTimeIncrement[i] = out.lengthOfTimeRange[i + 1] * 3600;
        }
    }

    return out;
}

}  // namespace metkit::mars2grib::backend::concepts_::impl

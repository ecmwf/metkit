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
/// @file DomainUtils.h
/// @brief Shared normalized-value primitives used by domain and shape builders.
///
/// This header resolves primitive durations required by several cases. It does not
/// construct complete domains and does not select a domain classification.
///
/// Every function catches all failures and rethrows `Mars2GribModelException`
/// directly. Functions receiving input attach `input.to_json()`.
///
/// @ingroup mars2grib_product_time_spec_detail
///
#pragma once

#include <limits>

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/timespan.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ForecastLeadUtils.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail {

///
/// @brief Retrieve the normalized non-seasonal forecast lead.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return Resolved `step` duration.
/// @throws Mars2GribModelException If the normalized `step` is missing.
///
inline metkit::mars2grib::backend::deductions::TimeDuration resolvedForecastStep(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasResolvedStep = input.step.has_value();

        if (!hasResolvedStep) {
            throw Mars2GribModelException("Non-seasonal forecast-domain construction requires a resolved step",
                                          input.to_json(), Here());
        }

        return *input.step;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to retrieve the non-seasonal ProductTimeSpec forecast lead", input.to_json(), Here()));
    }
}

///
/// @brief Retrieve the normalized seasonal forecast lead expressed in calendar months.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return Resolved `fcmonth` duration represented as `{length, Month}`.
/// @throws Mars2GribModelException If `fcmonth` is missing or locally invalid.
///
inline metkit::mars2grib::backend::deductions::TimeDuration resolvedSeasonalForecastLead(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool seasonalInputIsPresent = product_time_spec::detail::isSeasonal(input);

        if (!seasonalInputIsPresent) {
            throw Mars2GribModelException("Seasonal forecast-domain construction requires seasonal input semantics",
                                          input.to_json(), Here());
        }

        const long fcmonth = *input.marsFcmonth;

        if (fcmonth <= 0) {
            throw Mars2GribModelException("Seasonal forecast-domain construction requires a strictly positive fcmonth",
                                          input.to_json(), Here());
        }

        return TimeDuration{fcmonth, TimeUnit::Month};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to retrieve the seasonal ProductTimeSpec forecast lead",
                                                       input.to_json(), Here()));
    }
}

///
/// @brief Retrieve a duration-valued normalized timespan.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return Timespan represented as seconds.
/// @throws Mars2GribModelException If no duration value is available.
///
inline metkit::mars2grib::backend::deductions::TimeDuration timespanDuration(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::Timespan;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {

        const auto timespanKind = input.timespan.kind;

        if (timespanKind != TimespanKind::Duration) {
            throw Mars2GribModelException("Timespan is not duration-valued", input.to_json(), Here());
        }

        const bool hasDuration = input.timespan.duration.has_value();
        if (!hasDuration) {
            throw Mars2GribModelException("Duration-valued timespan does not contain a duration", input.to_json(),
                                          Here());
        }
        const auto duration = input.timespan.duration.value();

        long timespanInSeconds = convertToSeconds(duration);
        return TimeDuration{timespanInSeconds, TimeUnit::Second};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to retrieve ProductTimeSpec timespan duration", input.to_json(), Here()));
    }
}

///
/// @brief Resolve the outer support range required by normal domain builders.
///
/// Resolution follows source semantics before shape construction:
/// - instant products use zero;
/// - from-start products use the non-seasonal forecast lead;
/// - synoptic products use one calendar month;
/// - products with `stattype` blocks use the outermost block range;
/// - remaining duration-valued products use `timespan`.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return Outer range used to place normal forecast or analysis domains.
/// @throws Mars2GribModelException If no supported range source is available.
///
inline metkit::mars2grib::backend::deductions::TimeDuration resolveOuterDomainRange(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::Timespan;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isInstant =
            input.innerMostTypeOfStatisticalProcessing == tables::TypeOfStatisticalProcessing::Missing;
        const bool isFromStart = input.timespan.kind == TimespanKind::FromStart;

        const bool isSynoptic            = input.isSynoptic;
        const bool hasOuterStattypeBlock = !input.stattype.empty();
        const bool hasDurationTimespan   = input.timespan.kind == TimespanKind::Duration;

        if (isInstant) {
            return metkit::mars2grib::utils::time_arithmetic::zeroDuration();
        }
        if (isFromStart) {
            return resolvedForecastStep(input);
        }
        if (isSynoptic) {
            return TimeDuration{1, TimeUnit::Month};
        }
        if (hasOuterStattypeBlock) {
            return input.stattype.front().timeRange;
        }
        if (hasDurationTimespan) {
            return timespanDuration(input);
        }

        throw Mars2GribModelException("No outer domain range can be resolved from normalized input", input.to_json(),
                                      Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to resolve ProductTimeSpec outer domain range", input.to_json(), Here()));
    }
}

///
/// @brief Resolve the outer support range required by the seasonal forecast domain builder.
///
/// Resolution follows the same source semantics as the normal domain path, but
/// keeps the seasonal from-start branch explicitly separate so future seasonal
/// outer-range changes remain local to the seasonal forecast domain.
///
/// Current seasonal resolution rules are:
/// - instant products use zero;
/// - from-start products use the seasonal forecast lead;
/// - synoptic products use one calendar month;
/// - products with `stattype` blocks use the outermost block range;
/// - remaining duration-valued products use `timespan`.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return Outer range used to place the seasonal forecast domain.
/// @throws Mars2GribModelException If no supported range source is available.
///
inline metkit::mars2grib::backend::deductions::TimeDuration resolveSeasonalForecastOuterDomainRange(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isInstant =
            input.innerMostTypeOfStatisticalProcessing == tables::TypeOfStatisticalProcessing::Missing;
        const bool isFromStart = input.timespan.kind == TimespanKind::FromStart;

        const bool isSynoptic            = input.isSynoptic;
        const bool hasOuterStattypeBlock = !input.stattype.empty();
        const bool hasDurationTimespan   = input.timespan.kind == TimespanKind::Duration;

        if (isInstant) {
            return metkit::mars2grib::utils::time_arithmetic::zeroDuration();
        }
        if (isFromStart) {
            return resolvedSeasonalForecastLead(input);
        }
        if (isSynoptic) {
            return TimeDuration{1, tables::TimeUnit::Month};
        }
        if (hasOuterStattypeBlock) {
            return input.stattype.front().timeRange;
        }
        if (hasDurationTimespan) {
            return timespanDuration(input);
        }

        throw Mars2GribModelException("No seasonal forecast outer domain range can be resolved from normalized input",
                                      input.to_json(), Here());
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to resolve the seasonal ProductTimeSpec outer domain range", input.to_json(), Here()));
    }
}

///
/// @brief Compute the signed whole-hour offset from one datetime to another.
///
/// The result is positive when `targetDateTime` follows `referenceDateTime`,
/// negative when it precedes it, and zero when both datetimes are equal.
///
/// @param[in] referenceDateTime Datetime from which the offset is measured.
/// @param[in] targetDateTime Datetime whose signed whole-hour offset is needed.
/// @return Signed whole-hour offset from `referenceDateTime` to `targetDateTime`.
/// @throws Mars2GribModelException If the elapsed duration is not an exact
///         whole number of hours.
///
inline long offsetHoursFromReference(const eckit::DateTime& referenceDateTime, const eckit::DateTime& targetDateTime) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;
    using metkit::mars2grib::utils::time_arithmetic::durationBetween;

    try {
        const bool targetFollowsReference = targetDateTime >= referenceDateTime;
        const auto elapsedDuration =
            targetFollowsReference ? durationBetween(referenceDateTime, targetDateTime)
                                   : durationBetween(targetDateTime, referenceDateTime);
        const long elapsedSeconds = convertToSeconds(elapsedDuration);

        if (elapsedSeconds % 3600L != 0) {
            throw Mars2GribModelException("Reference-relative offset is not an exact whole number of hours", Here());
        }

        const long elapsedHours = elapsedSeconds / 3600L;

        if (targetFollowsReference) {
            return elapsedHours;
        }

        if (elapsedHours == std::numeric_limits<long>::min()) {
            throw Mars2GribModelException("Negative whole-hour offset is out of range for a long", Here());
        }

        return -elapsedHours;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to compute signed whole-hour offset from reference datetime", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

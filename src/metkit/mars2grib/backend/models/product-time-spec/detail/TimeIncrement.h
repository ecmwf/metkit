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
/// @file TimeIncrement.h
/// @brief Cross-cutting time-increment semantics shared by ProductTimeSpec shapes.
///
/// This internal header contains the shared ProductTimeSpec logic for
/// `timeIncrement` and `typeOfTimeIncrement`. It must not hide case-level
/// control flow that belongs in anchor, domain, or shape leaf builders.
///
/// Every function has a documented contract, catches all failures, and rethrows
/// `Mars2GribModelException` directly at the function boundary. Functions that
/// receive `ProductTimeSpecInput` attach `input.to_json()`; lower-level
/// functions use the location-only constructor.
///
/// @ingroup mars2grib_product_time_spec_detail
///
#pragma once

#include <limits>

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"

namespace metkit::mars2grib::backend::models::product_time_spec::detail {

/**
 * @brief Return the canonical value used when a time increment is semantically missing.
 *
 * The ProductTimeSpec representation stores a zero-second duration together with
 * `TypeOfTimeIntervals::Missing` when the increment is absent.
 *
 * @return Canonical missing-increment duration.
 * @throws Mars2GribModelException If construction of the duration unexpectedly fails.
 */
inline metkit::mars2grib::backend::deductions::TimeDuration missingIncrement() {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return {0, TimeUnit::Second};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `missingIncrement`", Here()));
    }
}

/**
 * @brief Return true when a duration represents exactly one elapsed hour.
 *
 * Both `{Hour, 1}` and `{Second, 3600}` are accepted because normalized inputs may
 * preserve either unit.
 *
 * @param[in] input Normalized input used to enrich exception diagnostics.
 * @param[in] duration Duration to inspect.
 * @return `true` when the duration is exactly one hour.
 * @throws Mars2GribModelException If evaluation unexpectedly fails.
 */
inline bool isOneHour(const ProductTimeSpecInput& input,
                      const metkit::mars2grib::backend::deductions::TimeDuration& duration) {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isOneHourInHours   = duration.unit == TimeUnit::Hour && duration.length == 1;
        const bool isOneHourInSeconds = duration.unit == TimeUnit::Second && duration.length == 3600;

        return isOneHourInHours || isOneHourInSeconds;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `isOneHour`", input.to_json(), Here()));
    }
}

/**
 * @brief Convert an elapsed duration to seconds for increment validation.
 *
 * Calendar-month ranges are intentionally rejected because their length cannot be
 * converted to seconds without a concrete calendar interval.
 *
 * @param[in] input Normalized input used to enrich exception diagnostics.
 * @param[in] duration Duration to convert.
 * @return Duration expressed in seconds.
 * @throws Mars2GribModelException For unsupported or calendar units.
 */
inline long durationInSecondsForValidation(const ProductTimeSpecInput& input,
                                           const metkit::mars2grib::backend::deductions::TimeDuration& duration) {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::detail::checkedSecondsFromUnits;

    try {
        long long seconds = 0;

        switch (duration.unit) {
            case TimeUnit::Second:
                return duration.length;
            case TimeUnit::Hour:
                seconds = checkedSecondsFromUnits(duration.length, 3600LL);
                break;
            case TimeUnit::Day:
                seconds = checkedSecondsFromUnits(duration.length, 86400LL);
                break;
            case TimeUnit::Month:
                throw Mars2GribModelException("Month-range increment validation requires calendar-aware comparison",
                                              input.to_json(), Here());
            default:
                throw Mars2GribModelException(
                    "Unsupported metkit::mars2grib::backend::deductions::TimeDuration unit during increment validation",
                    input.to_json(), Here());
        }

        if (seconds > static_cast<long long>(std::numeric_limits<long>::max()) ||
            seconds < static_cast<long long>(std::numeric_limits<long>::min())) {
            throw Mars2GribModelException("Increment-validation duration in seconds is out of range for a long",
                                          input.to_json(), Here());
        }

        return static_cast<long>(seconds);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `durationInSecondsForValidation`", input.to_json(), Here()));
    }
}

/**
 * @brief Return the GRIB missing sentinel for typeOfTimeIncrement.
 * @return `TypeOfTimeIntervals::Missing`.
 * @throws Mars2GribModelException If obtaining the sentinel unexpectedly fails.
 */
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals missingTypeOfTimeIncrement() {
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return TypeOfTimeIntervals::Missing;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `missingTypeOfTimeIncrement`", Here()));
    }
}

/**
 * @brief Return GRIB typeOfTimeIncrement value 2 used by forecast semantics.
 * @return GRIB code-table value 2.
 * @throws Mars2GribModelException If conversion unexpectedly fails.
 */
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals forecastTypeOfTimeIncrement() {
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return TypeOfTimeIntervals::SameForecastTimeStartIncremented;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `forecastTypeOfTimeIncrement`", Here()));
    }
}

/**
 * @brief Return GRIB typeOfTimeIncrement value 1 used by analysis semantics.
 * @return GRIB code-table value 1.
 * @throws Mars2GribModelException If conversion unexpectedly fails.
 */
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals analysisTypeOfTimeIncrement() {
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return TypeOfTimeIntervals::SameStartTimeForecastIncremented;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `analysisTypeOfTimeIncrement`", Here()));
    }
}

/**
 * @brief Resolve typeOfTimeIncrement for one IFS canonical window.
 *
 * Forecast windows use value 2. Analysis windows normally use value 1. The only
 * exception is the innermost one-hour window of an analysis multi-loop product,
 * which uses value 2.
 *
 * @param[in] input Normalized input containing analysis/forecast classification.
 * @param[in] isMultiLoop `true` when the final shape contains multiple windows.
 * @param[in] isInnermost `true` for the innermost canonical window.
 * @param[in] timeRange Canonical range of the inspected window.
 * @return GRIB typeOfTimeIncrement for the window.
 * @throws Mars2GribModelException For unsupported `Other` semantics.
 */
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals typeOfTimeIncrementForWindow(
    const ProductTimeSpecInput& input, bool isMultiLoop, bool isInnermost,
    const metkit::mars2grib::backend::deductions::TimeDuration& timeRange) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isForecast =
            input.simulationType == metkit::mars2grib::backend::deductions::SimulationType::Forecast;
        const bool isAnalysis =
            input.simulationType == metkit::mars2grib::backend::deductions::SimulationType::Analysis;
        const bool isOneHourInnermostAnalysisLoop =
            isAnalysis && isMultiLoop && isInnermost && isOneHour(input, timeRange);

        if (isForecast || isOneHourInnermostAnalysisLoop) {
            return forecastTypeOfTimeIncrement();
        }
        if (isAnalysis) {
            return analysisTypeOfTimeIncrement();
        }

        throw Mars2GribModelException("typeOfTimeIncrement cannot be assigned to AnalysisOrForecast::Other",
                                      input.to_json(), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `typeOfTimeIncrementForWindow`", input.to_json(), Here()));
    }
}

/**
 * @brief Fully resolved semantics of the innermost time increment.
 */
struct ResolvedInnerIncrement {
    metkit::mars2grib::backend::deductions::TimeDuration timeIncrement;
    metkit::mars2grib::backend::tables::TypeOfTimeIntervals typeOfTimeIncrement;
};

/**
 * @brief Validate an explicit or defaulted increment against the innermost range.
 *
 * The increment must be positive and may not exceed a non-calendar innermost
 * range. A zero-length from-start field may bypass the range comparison when that
 * special case has already been accepted by the relevant shape builder.
 *
 * @param[in] input Normalized input used for diagnostics.
 * @param[in] incrementInSeconds Increment to validate.
 * @param[in] innerRange Innermost canonical time range.
 * @param[in] allowZeroLengthFromStart Whether the zero-length from-start exception applies.
 * @throws Mars2GribModelException If validation fails.
 */
inline void validateExplicitIncrement(const ProductTimeSpecInput& input, long incrementInSeconds,
                                      const metkit::mars2grib::backend::deductions::TimeDuration& innerRange,
                                      bool allowZeroLengthFromStart) {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isPositive                   = incrementInSeconds > 0;
        const bool isAllowedZeroLengthFromStart = innerRange.length == 0 && allowZeroLengthFromStart;
        const bool isCalendarMonth              = innerRange.unit == TimeUnit::Month;

        if (!isPositive) {
            throw Mars2GribModelException("Explicit or defaulted timeIncrementInSeconds must be positive",
                                          input.to_json(), Here());
        }
        if (isAllowedZeroLengthFromStart) {
            return;
        }
        if (!isCalendarMonth && incrementInSeconds > durationInSecondsForValidation(input, innerRange)) {
            throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range", input.to_json(),
                                          Here());
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `validateExplicitIncrement`", input.to_json(), Here()));
    }
}

/**
 * @brief Deduce a missing IFS increment when defaulting is enabled.
 *
 * This function is the single extension point for the intentionally complex
 * default-increment algorithm. The current draft consumes the normalized
 * `defaultTimeIncrementInSeconds` option; the final implementation may inspect
 * additional input and domain facts without changing any shape builder.
 *
 * @param[in] input Normalized input and embedded options.
 * @param[in] domain Already resolved absolute domain.
 * @param[in] innerRange Innermost canonical time range.
 * @return Positive default increment expressed in seconds.
 * @throws Mars2GribModelException If no valid default can be deduced.
 */
inline long deduceDefaultTimeIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const metkit::mars2grib::backend::deductions::TimeDuration& innerRange) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)domain;
        (void)innerRange;

        throw Mars2GribModelException("Default time-increment deduction not implemented", input.to_json(), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `deduceDefaultTimeIncrement`", input.to_json(), Here()));
    }
}

/**
 * @brief Resolve explicit, missing, or defaulted IFS innermost increment semantics.
 *
 * Resolution follows one of three paths:
 *
 * - explicit source value: validate and use it;
 * - missing source value with defaulting disabled: encode a missing increment;
 * - missing source value with defaulting enabled: call `deduceDefaultTimeIncrement`,
 *   validate the result, and use it.
 *
 * @param[in] input Normalized input and embedded options.
 * @param[in] domain Already resolved absolute domain.
 * @param[in] innerRange Innermost canonical time range.
 * @param[in] isMultiLoop Whether the final shape contains multiple windows.
 * @param[in] allowZeroLengthFromStart Whether the zero-length from-start exception applies.
 * @return Fully resolved innermost increment semantics.
 * @throws Mars2GribModelException If explicit or defaulted values are invalid.
 */
inline ResolvedInnerIncrement resolveIfsInnerIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const metkit::mars2grib::backend::deductions::TimeDuration& innerRange, bool isMultiLoop,
    bool allowZeroLengthFromStart = false) {
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        const bool hasExplicitIncrement = input.timeIncrement.has_value();
        const bool defaultingIsEnabled  = input.allowDefaultTimeIncrement;

        if (hasExplicitIncrement) {
            auto incrementInSeconds = convertToSeconds(*input.timeIncrement);
            validateExplicitIncrement(input, incrementInSeconds, innerRange, allowZeroLengthFromStart);

            return ResolvedInnerIncrement{
                metkit::mars2grib::backend::deductions::TimeDuration{incrementInSeconds, TimeUnit::Second},
                typeOfTimeIncrementForWindow(input, isMultiLoop, true, innerRange)};
        }

        if (!defaultingIsEnabled) {
            return ResolvedInnerIncrement{missingIncrement(), missingTypeOfTimeIncrement()};
        }

        const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, domain, innerRange);

        validateExplicitIncrement(input, defaultIncrementInSeconds, innerRange, allowZeroLengthFromStart);

        return ResolvedInnerIncrement{
            metkit::mars2grib::backend::deductions::TimeDuration{defaultIncrementInSeconds, TimeUnit::Second},
            typeOfTimeIncrementForWindow(input, isMultiLoop, true, innerRange)};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `resolveIfsInnerIncrement`", input.to_json(), Here()));
    }
}

/**
 * @brief Resolve the intrinsic twenty-four-hour increment of synoptic analysis.
 *
 * A source increment is redundant. It is accepted only when redundant values are
 * enabled and its value is exactly 86400 seconds. The returned semantic increment
 * is always twenty-four hours with typeOfTimeIncrement value 1.
 *
 * @param[in] input Normalized synoptic input and embedded options.
 * @return Intrinsic synoptic increment semantics.
 * @throws Mars2GribModelException If a redundant value is forbidden or wrong.
 */
inline ResolvedInnerIncrement resolveSynopticIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        constexpr long expectedIncrementInSeconds = 86400L;

        const bool hasExplicitIncrement        = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;
        const bool explicitIncrementHasExpectedValue =
            !hasExplicitIncrement || convertToSeconds(*input.timeIncrement) == expectedIncrementInSeconds;

        if (hasExplicitIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Synoptic timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(),
                Here());
        }
        if (!explicitIncrementHasExpectedValue) {
            throw Mars2GribModelException("Synoptic timeIncrementInSeconds must equal 86400", input.to_json(), Here());
        }

        return ResolvedInnerIncrement{metkit::mars2grib::utils::time_arithmetic::twentyFourHours(),
                                      analysisTypeOfTimeIncrement()};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `resolveSynopticIncrement`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate a redundant source increment attached to an instant product.
 *
 * Instant products do not require an increment. A present source value is
 * therefore accepted only when redundant increments are explicitly enabled.
 *
 * @param[in] input Normalized instant input and embedded options.
 * @throws Mars2GribModelException If a redundant value is forbidden.
 */
inline void validateInstantIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasIncrement                = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;

        if (hasIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Instant timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(),
                Here());
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `validateInstantIncrement`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::detail

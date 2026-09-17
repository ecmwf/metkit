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
/// receive `ProductTimeSpecInput` attach `input.to_json(cntx)`; lower-level
/// functions use the location-only constructor.
///
/// @ingroup mars2grib_product_time_spec_detail
///
#pragma once

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
 * @param[in,out] cntx Profiling context.
 * @return Canonical missing-increment duration.
 * @throws Mars2GribModelException If construction of the duration unexpectedly fails.
 */
template <class Cntx_t>
inline metkit::mars2grib::backend::deductions::TimeDuration missingIncrement(Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            metkit::mars2grib::backend::deductions::TimeDuration result{0, TimeUnit::Missing};
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
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
 * @param[in,out] cntx Profiling context.
 * @return `true` when the duration is exactly one hour.
 * @throws Mars2GribModelException If evaluation unexpectedly fails.
 */
template <class Cntx_t>
inline bool isOneHour(const ProductTimeSpecInput& input,
                      const metkit::mars2grib::backend::deductions::TimeDuration& duration, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isOneHourInHours   = duration.unit == TimeUnit::Hour && duration.length == 1;
        const bool isOneHourInSeconds = duration.unit == TimeUnit::Second && duration.length == 3600;

        {
            bool result = isOneHourInHours || isOneHourInSeconds;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `isOneHour`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
 * @param[in,out] cntx Profiling context.
 * @return Duration expressed in seconds.
 * @throws Mars2GribModelException For unsupported or calendar units.
 */
template <class Cntx_t>
inline long durationInSecondsForValidation(const ProductTimeSpecInput& input,
                                           const metkit::mars2grib::backend::deductions::TimeDuration& duration, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::detail::checkedSecondsFromUnits;

    try {
        long long seconds = 0;

        switch (duration.unit) {
            case TimeUnit::Second:
                {
                    long result = duration.length;
                    metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            case TimeUnit::Hour:
                seconds = checkedSecondsFromUnits(duration.length, 3600LL, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
                break;
            case TimeUnit::Day:
                seconds = checkedSecondsFromUnits(duration.length, 86400LL, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
                break;
            case TimeUnit::Month:
                throw Mars2GribModelException("Month-range increment validation requires calendar-aware comparison",
                                              input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
            default:
                throw Mars2GribModelException(
                    "Unsupported metkit::mars2grib::backend::deductions::TimeDuration unit during increment validation",
                    input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        if (seconds > static_cast<long long>(std::numeric_limits<long>::max()) ||
            seconds < static_cast<long long>(std::numeric_limits<long>::min())) {
            throw Mars2GribModelException("Increment-validation duration in seconds is out of range for a long",
                                          input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            long result = static_cast<long>(seconds);
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `durationInSecondsForValidation`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

/**
 * @brief Return the GRIB missing sentinel for typeOfTimeIncrement.
 * @param[in,out] cntx Profiling context.
 * @return `TypeOfTimeIntervals::Missing`.
 * @throws Mars2GribModelException If obtaining the sentinel unexpectedly fails.
 */
template <class Cntx_t>
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals missingTypeOfTimeIncrement(Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            metkit::mars2grib::backend::tables::TypeOfTimeIntervals result = TypeOfTimeIntervals::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `missingTypeOfTimeIncrement`", Here()));
    }
}

/**
 * @brief Return GRIB typeOfTimeIncrement value 2 used by forecast semantics.
 * @param[in,out] cntx Profiling context.
 * @return GRIB code-table value 2.
 * @throws Mars2GribModelException If conversion unexpectedly fails.
 */
template <class Cntx_t>
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals forecastTypeOfTimeIncrement(Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            metkit::mars2grib::backend::tables::TypeOfTimeIntervals result = TypeOfTimeIntervals::SameForecastTimeStartIncremented;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `forecastTypeOfTimeIncrement`", Here()));
    }
}

/**
 * @brief Return GRIB typeOfTimeIncrement value 1 used by analysis semantics.
 * @param[in,out] cntx Profiling context.
 * @return GRIB code-table value 1.
 * @throws Mars2GribModelException If conversion unexpectedly fails.
 */
template <class Cntx_t>
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals analysisTypeOfTimeIncrement(Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            metkit::mars2grib::backend::tables::TypeOfTimeIntervals result = TypeOfTimeIntervals::SameStartTimeForecastIncremented;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
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
 * @param[in,out] cntx Profiling context.
 * @return GRIB typeOfTimeIncrement for the window.
 * @throws Mars2GribModelException For unsupported `Other` semantics.
 */
template <class Cntx_t>
inline metkit::mars2grib::backend::tables::TypeOfTimeIntervals typeOfTimeIncrementForWindow(
    const ProductTimeSpecInput& input, bool isMultiLoop, bool isInnermost,
    const metkit::mars2grib::backend::deductions::TimeDuration& timeRange, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isForecast =
            input.simulationType == metkit::mars2grib::backend::deductions::SimulationType::Forecast;
        const bool isAnalysis =
            input.simulationType == metkit::mars2grib::backend::deductions::SimulationType::Analysis;
        const bool isOneHourInnermostAnalysisLoop =
            isAnalysis && isMultiLoop && isInnermost && isOneHour(input, timeRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

        // if (isForecast || isOneHourInnermostAnalysisLoop) {
        {
            metkit::mars2grib::backend::tables::TypeOfTimeIntervals result = forecastTypeOfTimeIncrement(metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        //}
        // if (isAnalysis) {
        //    metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        //    return analysisTypeOfTimeIncrement(cntx);
        //}

        throw Mars2GribModelException("typeOfTimeIncrement cannot be assigned to AnalysisOrForecast::Other",
                                      input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `typeOfTimeIncrementForWindow`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
 * @param[in,out] cntx Profiling context.
 * @throws Mars2GribModelException If validation fails.
 */
template <class Cntx_t>
inline void validateExplicitIncrement(const ProductTimeSpecInput& input, long incrementInSeconds,
                                      const metkit::mars2grib::backend::deductions::TimeDuration& innerRange,
                                      bool allowZeroLengthFromStart, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isPositive                   = incrementInSeconds > 0;
        const bool isAllowedZeroLengthFromStart = innerRange.length == 0 && allowZeroLengthFromStart;
        const bool isCalendarMonth              = innerRange.unit == TimeUnit::Month;

        if (!isPositive) {
            throw Mars2GribModelException("Explicit or defaulted timeIncrementInSeconds must be positive",
                                          input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }
        if (isAllowedZeroLengthFromStart) {
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return;
        }
        if (!isCalendarMonth && incrementInSeconds > durationInSecondsForValidation(input, innerRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
            throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
                                          Here());
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `validateExplicitIncrement`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }

    metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    return;
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
 * @param[in] innerRange Innermost canonical time range.
 * @param[in,out] cntx Profiling context.
 * @return Positive default increment expressed in seconds.
 * @throws Mars2GribModelException If no valid default can be deduced.
 */
template <class Cntx_t>
inline long deduceDefaultTimeIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::deductions::TimeDuration& innerRange, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)innerRange;

        throw Mars2GribModelException("Default time-increment deduction not implemented", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `deduceDefaultTimeIncrement`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
 * @param[in] innerRange Innermost canonical time range.
 * @param[in] isMultiLoop Whether the final shape contains multiple windows.
 * @param[in] allowZeroLengthFromStart Whether the zero-length from-start exception applies.
 * @param[in,out] cntx Profiling context.
 * @return Fully resolved innermost increment semantics.
 * @throws Mars2GribModelException If explicit or defaulted values are invalid.
 */
template <class Cntx_t>
inline ResolvedInnerIncrement resolveIfsInnerIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::deductions::TimeDuration& innerRange, bool isMultiLoop,
    bool allowZeroLengthFromStart, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        const bool hasExplicitIncrement = input.timeIncrement.has_value();
        const bool defaultingIsEnabled  = input.allowDefaultTimeIncrement;

        if (hasExplicitIncrement) {
            auto incrementInSeconds = convertToSeconds(*input.timeIncrement, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            validateExplicitIncrement(input, incrementInSeconds, innerRange, allowZeroLengthFromStart, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

            {
                ResolvedInnerIncrement result = ResolvedInnerIncrement{
                metkit::mars2grib::backend::deductions::TimeDuration{incrementInSeconds, TimeUnit::Second},
                typeOfTimeIncrementForWindow(input, isMultiLoop, true, innerRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here()))};
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }

        if (!defaultingIsEnabled) {
            {
                ResolvedInnerIncrement result = ResolvedInnerIncrement{missingIncrement(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), missingTypeOfTimeIncrement(metkit::mars2grib::utils::profiling::callSite(cntx, Here()))};
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }

        const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, innerRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

        validateExplicitIncrement(input, defaultIncrementInSeconds, innerRange, allowZeroLengthFromStart, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

        {
            ResolvedInnerIncrement result = ResolvedInnerIncrement{
            metkit::mars2grib::backend::deductions::TimeDuration{defaultIncrementInSeconds, TimeUnit::Second},
            typeOfTimeIncrementForWindow(input, isMultiLoop, true, innerRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here()))};
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `resolveIfsInnerIncrement`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
 * @param[in,out] cntx Profiling context.
 * @return Intrinsic synoptic increment semantics.
 * @throws Mars2GribModelException If a redundant value is forbidden or wrong.
 */
template <class Cntx_t>
inline ResolvedInnerIncrement resolveSynopticIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        constexpr long expectedIncrementInSeconds = 86400L;

        const bool hasExplicitIncrement        = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;
        const bool explicitIncrementHasExpectedValue =
            !hasExplicitIncrement || convertToSeconds(*input.timeIncrement, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) == expectedIncrementInSeconds;

        if (hasExplicitIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Synoptic timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
                Here());
        }
        if (!explicitIncrementHasExpectedValue) {
            throw Mars2GribModelException("Synoptic timeIncrementInSeconds must equal 86400", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            ResolvedInnerIncrement result = ResolvedInnerIncrement{metkit::mars2grib::utils::time_arithmetic::twentyFourHours(metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
                                      analysisTypeOfTimeIncrement(metkit::mars2grib::utils::profiling::callSite(cntx, Here()))};
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `resolveSynopticIncrement`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

/**
 * @brief Validate a redundant source increment attached to an instant product.
 *
 * Instant products do not require an increment. A present source value is
 * therefore accepted only when redundant increments are explicitly enabled.
 *
 * @param[in] input Normalized instant input and embedded options.
 * @param[in,out] cntx Profiling context.
 * @throws Mars2GribModelException If a redundant value is forbidden.
 */
template <class Cntx_t>
inline void validateInstantIncrement(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasIncrement                = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;

        if (hasIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Instant timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
                Here());
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `validateInstantIncrement`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }

    metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    return;
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::detail

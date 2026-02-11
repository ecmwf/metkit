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
/// @file statisticsOp.h
/// @brief Implementation of the GRIB `statistics` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **statistics concept** within the mars2grib backend.
///
/// The `statistics` concept is responsible for encoding GRIB metadata
/// related to statistical processing over time, including:
/// - type of statistical processing (e.g. mean, accumulation, extremes),
/// - time range structure,
/// - time increment and span,
/// - start and end steps for statistical intervals.
///
/// The concept operates exclusively in the *Product Definition Section*
/// (Section 4) and is executed across multiple encoding stages
/// (`Allocate`, `Preset`, `Runtime`), each contributing a well-defined
/// subset of the GRIB keys.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `statisticsApplicable`
/// - Stage-dependent encoding logic
/// - Centralized deduction of time descriptors from MARS metadata
/// - Strict validation of GRIB structural constraints
/// - Context-rich error handling
///
/// @note
/// Support for multiple time ranges is currently **incomplete** and
/// explicitly rejected at both preset and runtime stages.
/// This limitation is documented and enforced at runtime.
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of
/// `concepts` to avoid conflicts with the C++20 `concepts` language feature.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/forecastTimeInSeconds.h"
#include "metkit/mars2grib/backend/deductions/numberOfTimeRanges.h"
#include "metkit/mars2grib/backend/deductions/statisticsDescriptor.h"
#include "metkit/mars2grib/backend/deductions/timeIncrementInSeconds.h"
#include "metkit/mars2grib/backend/deductions/timeSpanInSeconds.h"

// checks
#include "metkit/mars2grib/backend/checks/checkStatisticsProductDefinitionSection.h"

// Tables
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"

// Deduction helpers
#include "metkit/mars2grib/backend/deductions/detail/timeUtils.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `statistics` concept.
///
/// This predicate determines whether the `statistics` concept is applicable
/// for a given encoding stage, GRIB section, and concept variant.
///
/// The concept is applicable for:
/// - any encoding stage
/// - the *Product Definition Section* (Section 4)
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index
/// @tparam Variant Statistics concept variant
///
/// @return `true` if the concept is applicable, `false` otherwise.
///
template <std::size_t Stage, std::size_t Section, StatisticsType Variant>
constexpr bool statisticsApplicable() {
    return (Section == SecProductDefinitionSection);
}


///
/// @brief Execute the `statistics` concept operation.
///
/// This function implements the runtime logic of the GRIB `statistics` concept.
/// Depending on the encoding stage, it performs the following actions:
///
/// ### StageAllocate
/// - Validates that the Product Definition Section supports statistics.
/// - Encodes the number of statistical time ranges.
///
/// ### StagePreset
/// - Encodes the statistical processing type.
/// - Encodes time unit metadata for ranges and increments.
/// - Handles special cases where the time increment is missing
/// (legacy AIFS behavior).
/// - Rejects unsupported multi-range configurations.
///
/// ### StageRuntime
/// - Computes and encodes `startStep` and `endStep`.
/// - Resolves time span and forecast step from MARS metadata.
/// - Explicitly rejects multiple time ranges.
///
/// The concept relies on multiple time-related deductions and helper
/// utilities to interpret MARS time semantics consistently.
///
/// @tparam Stage     Encoding stage (compile-time constant)
/// @tparam Section   GRIB section index
/// @tparam Variant   Statistics concept variant
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the GRIB output dictionary
///
/// @param[in]  mars MARS input dictionary
/// @param[in]  par  Parameter dictionary
/// @param[in]  opt  Options dictionary
/// @param[out] out  Output GRIB dictionary to be populated
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
/// If:
/// - the Product Definition Section is incompatible with statistics
/// - unsupported multi-range configurations are detected
/// - any deduction or encoding step fails
///
/// @note
/// - Time units are currently normalized to **hours** at GRIB level.
/// - Time increment handling contains legacy logic and known hacks.
/// - Multiple time ranges are not yet supported.
///
/// @see statisticsApplicable
/// @see deductions::numberOfTimeRanges
/// @see deductions::getTimeDescriptorFromMars_orThrow
///
template <std::size_t Stage, std::size_t Section, StatisticsType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void StatisticsOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;


    if constexpr (statisticsApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(statistics);

            // Global deduction used in multiple stages
            long numberOfTimeRangesVal = deductions::numberOfTimeRanges(mars, par);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Stage == StageAllocate) {

                // Checks/Validation
                validation::check_StatisticsProductDefinitionSection_or_throw(opt, out);

                // Encoding
                setMissing_or_throw(out, "hoursAfterDataCutoff");
                setMissing_or_throw(out, "minutesAfterDataCutoff");
                set_or_throw<long>(out, "numberOfTimeRanges", numberOfTimeRangesVal);
            }

            if constexpr (Stage == StagePreset) {

                // Deductions
                std::optional<long> timeIncrementInSecondsOpt = deductions::timeIncrementInSeconds_opt(mars, par);

                // Encoding
                set_or_throw<long>(out, "typeOfStatisticalProcessing", typeOfStatisticalProcessing<Variant>());
                set_or_throw<long>(out, "indicatorOfUnitOfTimeRange", static_cast<long>(TimeUnit::Hour));
                set_or_throw<long>(out, "indicatorOfUnitForTimeRange", static_cast<long>(TimeUnit::Hour));


                // HACK: handle special case for AIFS (MUL-227)
                if (numberOfTimeRangesVal == 1 && !timeIncrementInSecondsOpt.has_value()) {

                    // Encoding
                    set_or_throw<long>(
                        out, "typeOfTimeIncrement",
                        static_cast<long>(tables::TypeOfTimeIntervals::SameStartTimeForecastIncremented));
                    set_or_throw<long>(out, "indicatorOfUnitForTimeIncrement", static_cast<long>(TimeUnit::Missing));
                    set_or_throw<long>(out, "timeIncrement", 0L);
                }
                else {

                    // Encoding
                    set_or_throw<long>(
                        out, "typeOfTimeIncrement",
                        static_cast<long>(tables::TypeOfTimeIntervals::SameStartTimeForecastIncremented));
                    set_or_throw<long>(out, "indicatorOfUnitForTimeIncrement", static_cast<long>(TimeUnit::Second));
                    set_or_throw<long>(out, "timeIncrement", timeIncrementInSecondsOpt.value());


                    // Test WIP
                    deductions::StatisticalProcessing statsDesc = deductions::getTimeDescriptorFromMars_orThrow(
                        mars, par, opt, typeOfStatisticalProcessing<Variant>());

                    if (numberOfTimeRangesVal > 1) {
                        MARS2GRIB_CONCEPT_THROW(
                            statistics,
                            "`statistics` concept with multiple time ranges not yet supported at preset stage...");
                    }
                }
            }

            if constexpr (Stage == StageRuntime) {

                // Deductions
                long stepInHour     = deductions::resolve_ForecastTimeInSeconds_or_throw(mars, par, opt) / 3600;
                long timeSpanInHour = deductions::resolve_TimeSpanInSeconds_or_throw(mars, par, opt) / 3600;

                // Get the length of timestep in seconds
                std::optional<long> timeIncrementInSecondsOpt = deductions::timeIncrementInSeconds_opt(mars, par);

                long tmp       = stepInHour - timeSpanInHour;
                long startStep = (tmp >= 0) ? tmp : 0;
                long endStep   = stepInHour;

                // Encoding
                set_or_throw<long>(out, "startStep", startStep);
                set_or_throw<long>(out, "endStep", endStep);

                // Test WIP
                if (timeIncrementInSecondsOpt.has_value()) {
                    deductions::StatisticalProcessing statsDesc = deductions::getTimeDescriptorFromMars_orThrow(
                        mars, par, opt, typeOfStatisticalProcessing<Variant>());
                };


                if (numberOfTimeRangesVal > 1) {
                    MARS2GRIB_CONCEPT_THROW(
                        statistics,
                        "`statistics` concept with multiple time ranges not yet supported at runtime stage...");
                }
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(statistics, "Unable to set `statistics` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(statistics, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

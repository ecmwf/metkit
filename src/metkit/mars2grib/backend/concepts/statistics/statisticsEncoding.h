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
/// @file statisticsEncoding.h
/// @brief Implementation of the GRIB `statistics` concept operation.
///
/// The `statistics` concept encodes GRIB Section-4 keys describing
/// statistical processing over time (PDT 4.8 / 4.11). It runs in three
/// stages:
///
/// ### StageAllocate
/// - Validates that the Product Definition Section supports statistics.
/// - Encodes `numberOfTimeRanges` (from `ProductTime`).
/// - Sets `hoursAfterDataCutoff` / `minutesAfterDataCutoff` to missing.
///
/// ### StagePreset
/// - Computes the per-loop statistical processing descriptor from a
///   `ProductTime` and writes the 6 SoA vectors verbatim:
///     * `typeOfStatisticalProcessing`
///     * `typeOfTimeIncrement`
///     * `indicatorOfUnitForTimeRange`
///     * `lengthOfTimeRange`
///     * `indicatorOfUnitForTimeIncrement`
///     * `lengthOfTimeIncrement`
///   No instant / single-window / multi-window branching: a single uniform
///   path covers all cases (including the AIFS no-increment hack which is
///   handled inside `compute_StatisticalProcessing`).
///
/// ### StageRuntime
/// - Encodes the time-dependent keys from the resolved `ProductTime`:
///   - `forecastTime` (hours between `pt.referenceDateTime` and `pt.windowStart`)
///   - `<year|month|day|hour|minute|second>OfEndOfOverallTimeInterval` (from `pt.windowEnd`)
///
/// All temporal data is sourced exclusively from the `ProductTime`
/// produced by `resolve_ProductTime_or_throw` (§15 of `timeProducts.md`).
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/statistics/impl/statisticsDescriptor.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions (ProductTime + per-loop type-of-statistical-processing resolver)
#include "metkit/mars2grib/backend/deductions/productTime.h"
#include "metkit/mars2grib/backend/deductions/typeOfStatisticalProcessing.h"

// Checks
#include "metkit/mars2grib/backend/checks/checkStatisticsProductDefinitionSection.h"

// Tables
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `statistics` concept.
///
/// The concept is applicable for the *Product Definition Section* (Section 4)
/// at any encoding stage.
///
template <std::size_t Stage, std::size_t Section, StatisticsType Variant>
constexpr bool statisticsApplicable() {
    return (Section == SecProductDefinitionSection);
}


///
/// @brief Execute the `statistics` concept operation.
///
/// See file-level documentation for stage-by-stage semantics.
///
template <std::size_t Stage, std::size_t Section, StatisticsType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void StatisticsOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;


    if constexpr (statisticsApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(statistics);

            // =============================================================
            // StageAllocate
            // =============================================================
            if constexpr (Stage == StageAllocate) {

                auto pt = deductions::resolve_ProductTime_or_throw(mars, par, opt);

                // Checks/Validation
                validation::check_StatisticsProductDefinitionSection_or_throw(opt, out);

                // Encoding
                setMissing_or_throw(out, "hoursAfterDataCutoff");
                setMissing_or_throw(out, "minutesAfterDataCutoff");
                set_or_throw<long>(out, "numberOfTimeRanges", deductions::numberOfTimeRanges(pt));
            }

            // =============================================================
            // StagePreset
            //
            // Single uniform path. No branching for instant / single-window
            // / multi-window: `compute_StatisticalProcessing` handles all
            // cases (including the AIFS no-increment hack) internally.
            // =============================================================
            if constexpr (Stage == StagePreset) {

                auto pt    = deductions::resolve_ProductTime_or_throw(mars, par, opt);
                auto inner = typeOfStatisticalProcessingEnum<Variant>();
                auto types = deductions::resolve_TypeOfStatisticalProcessing_or_throw(inner, mars, par, opt);

                auto desc = impl::compute_StatisticalProcessing(pt, types);

                set_or_throw<std::vector<long>>(out, "typeOfStatisticalProcessing", desc.typeOfStatisticalProcessing);
                set_or_throw<std::vector<long>>(out, "typeOfTimeIncrement", desc.typeOfTimeIncrement);
                set_or_throw<std::vector<long>>(out, "indicatorOfUnitForTimeRange", desc.indicatorOfUnitForTimeRange);
                set_or_throw<std::vector<long>>(out, "lengthOfTimeRange", desc.lengthOfTimeRange);
                set_or_throw<std::vector<long>>(out, "indicatorOfUnitForTimeIncrement",
                                                desc.indicatorOfUnitForTimeIncrement);
                set_or_throw<std::vector<long>>(out, "lengthOfTimeIncrement", desc.lengthOfTimeIncrement);
            }

            // =============================================================
            // StageRuntime
            //
            // Time-dependent keys: forecastTime and end-of-interval date/time.
            // =============================================================
            if constexpr (Stage == StageRuntime) {
                auto pt = deductions::resolve_ProductTime_or_throw(mars, par, opt);

                // forecastTime is the distance in hours between reference time
                // and WindowBegin (pt.windowStart). It must be a non-negative
                // integer number of hours.
                const long deltaSeconds =
                    static_cast<long>(static_cast<eckit::Second>(pt.windowStart - pt.referenceDateTime));

                if (deltaSeconds < 0) {
                    throw Mars2GribConceptException(
                        std::string(statisticsName), std::string(statisticsTypeName<Variant>()), std::to_string(Stage),
                        std::to_string(Section), "statistics: forecastTime must be non-negative", Here());
                }

                if (deltaSeconds % 3600 != 0) {
                    throw Mars2GribConceptException(
                        std::string(statisticsName), std::string(statisticsTypeName<Variant>()), std::to_string(Stage),
                        std::to_string(Section), "statistics: forecastTime must be a whole number of hours", Here());
                }

                set_or_throw<long>(out, "forecastTime", deltaSeconds / 3600);

                const eckit::DateTime& end = pt.windowEnd;
                set_or_throw<long>(out, "yearOfEndOfOverallTimeInterval", end.date().year());
                set_or_throw<long>(out, "monthOfEndOfOverallTimeInterval", end.date().month());
                set_or_throw<long>(out, "dayOfEndOfOverallTimeInterval", end.date().day());
                set_or_throw<long>(out, "hourOfEndOfOverallTimeInterval", end.time().hours());
                set_or_throw<long>(out, "minuteOfEndOfOverallTimeInterval", end.time().minutes());
                set_or_throw<long>(out, "secondOfEndOfOverallTimeInterval", end.time().seconds());
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
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

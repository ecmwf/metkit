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
/// - Builds one final immutable `ProductTimeSpec`.
/// - Encodes `numberOfTimeRanges` from the corresponding
///   `StatisticsProductTimeSpec` transport.
/// - Sets `hoursAfterDataCutoff` / `minutesAfterDataCutoff` to missing.
///
/// ### StagePreset
/// - Builds one final immutable `ProductTimeSpec`.
/// - Builds one `StatisticsProductTimeSpec` transport from that model.
/// - Writes the 6 SoA vectors verbatim:
///     * `typeOfStatisticalProcessing`
///     * `typeOfTimeIncrement`
///     * `indicatorOfUnitForTimeRange`
///     * `lengthOfTimeRange`
///     * `indicatorOfUnitForTimeIncrement`
///     * `lengthOfTimeIncrement`
///   No instant / single-window / multi-window branching is needed in the
///   encoding layer because the transport already materializes the canonical
///   vectors.
///
/// ### StageRuntime
/// - Builds one final immutable `ProductTimeSpec`.
/// - Builds one `StatisticsProductTimeSpec` transport from that model.
/// - Encodes the time-dependent keys from that transport:
///   - `forecastTime`
///   - `<year|month|day|hour|minute|second>OfEndOfOverallTimeInterval`
///
/// All temporal data is sourced exclusively from the final immutable
/// `backend::models::product_time_spec::ProductTimeSpec` model through the
/// statistics-facing transport struct built by
/// `build_StatisticsProductTimeSpec_or_throw(...)`.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/statistics/impl/StatisticsProductTimeSpec.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/Profiling.h"

// Models (new product time implementation)
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/common.h"

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
    return (Section == SecProductDefinitionSection && Variant != StatisticsType::IndexProcessing);
}


///
/// @brief Execute the `statistics` concept operation.
///
/// See file-level documentation for stage-by-stage semantics.
///
/// All encoded temporal payload values and GRIB statistical-processing arrays
/// are sourced from `StatisticsProductTimeSpec`, which is itself built from one
/// final immutable `backend::models::product_time_spec::ProductTimeSpec`.
///
template <std::size_t Stage, std::size_t Section, StatisticsType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t, class Cntx_t>
void StatisticsOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out, Cntx_t& cntx) {
    utils::profiling::profileEnterConcept<Stage, Section, Variant>(cntx, Here());

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

                const auto spec = models::product_time_spec::ProductTimeSpec(typeOfStatisticalProcessingEnum<Variant>(),
                                                                             mars, par, opt, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                const auto pts  = impl::build_StatisticsProductTimeSpec_or_throw(spec, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));

                // Checks/Validation
                validation::check_StatisticsProductDefinitionSection_or_throw(opt, out, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));

                // Encoding
                setMissing_or_throw(out, "hoursAfterDataCutoff", utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                setMissing_or_throw(out, "minutesAfterDataCutoff", utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<long>(out, "numberOfTimeRanges", pts.numberOfTimeRanges, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
            }

            // =============================================================
            // StagePreset
            //
            // Single uniform path. No branching for instant / single-window /
            // multi-window: the resolved `StatisticsProductTimeSpec` transport
            // already materializes the canonical vectors.
            // =============================================================
            if constexpr (Stage == StagePreset) {

                const auto spec = models::product_time_spec::ProductTimeSpec(typeOfStatisticalProcessingEnum<Variant>(),
                                                                             mars, par, opt, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                const auto pts  = impl::build_StatisticsProductTimeSpec_or_throw(spec, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));

                set_or_throw<std::vector<long>>(out, "typeOfStatisticalProcessing", pts.typeOfStatisticalProcessing, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<std::vector<long>>(out, "typeOfTimeIncrement", pts.typeOfTimeIncrement, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<std::vector<long>>(out, "indicatorOfUnitForTimeRange", pts.indicatorOfUnitForTimeRange, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<std::vector<long>>(out, "lengthOfTimeRange", pts.lengthOfTimeRange, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<std::vector<long>>(out, "indicatorOfUnitForTimeIncrement",
                                                pts.indicatorOfUnitForTimeIncrement, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<std::vector<long>>(out, "timeIncrement", pts.lengthOfTimeIncrement, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
            }

            // =============================================================
            // StageRuntime
            //
            // Time-dependent keys: forecastTime and end-of-interval date/time.
            // =============================================================
            if constexpr (Stage == StageRuntime) {
                const auto spec = models::product_time_spec::ProductTimeSpec(typeOfStatisticalProcessingEnum<Variant>(),
                                                                             mars, par, opt, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                const auto pts  = impl::build_StatisticsProductTimeSpec_or_throw(spec, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));

                set_or_throw<long>(out, "forecastTime", pts.forecastTime.length, utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));

                const eckit::DateTime& end = pts.endOfOverallTimeInterval;
                set_or_throw<long>(out, "yearOfEndOfOverallTimeInterval", end.date().year(), utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<long>(out, "monthOfEndOfOverallTimeInterval", end.date().month(), utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<long>(out, "dayOfEndOfOverallTimeInterval", end.date().day(), utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<long>(out, "hourOfEndOfOverallTimeInterval", end.time().hours(), utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<long>(out, "minuteOfEndOfOverallTimeInterval", end.time().minutes(), utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
                set_or_throw<long>(out, "secondOfEndOfOverallTimeInterval", end.time().seconds(), utils::profiling::conceptCallSite<Stage, Section, Variant>(cntx, Here()));
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(statistics, "Unable to set `statistics` concept...");
        }

        // Successful operation
        utils::profiling::profileExitConcept<Stage, Section, Variant>(cntx, Here());
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(statistics, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

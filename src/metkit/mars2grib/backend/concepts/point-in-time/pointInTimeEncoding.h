/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file pointInTimeOp.h
 * @brief Implementation of the GRIB `pointInTime` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **pointInTime concept** within the mars2grib backend.
 *
 * The pointInTime concept is responsible for encoding GRIB keys that describe
 * the temporal reference of a product expressed as a *point in time*, i.e.
 * a forecast offset relative to the reference time.
 *
 * The concept operates across multiple encoding stages:
 * - **StageAllocate**: prepares time-related keys and marks unused fields as missing
 * - **StagePreset**: defines the unit of the time range (hours)
 * - **StageRuntime**: sets the actual forecast time value
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `pointInTimeApplicable`
 * - Runtime deduction of forecast time
 * - Stage-specific encoding logic
 * - Strict error handling with contextual concept exceptions
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of `concepts`
 * to avoid ambiguity and potential conflicts with the C++20 `concept` language
 * feature and related standard headers.
 *
 * This is a deliberate design choice and must not be changed.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/point-in-time/pointInTimeEnum.h"

// Tables
#include "metkit/mars2grib/backend/tables/timeUnits.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/forecastTimeInSeconds.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `pointInTime` concept.
 *
 * The default applicability enables this concept for the
 * *Product Definition Section* at all encoding stages:
 * - `StageAllocate`
 * - `StagePreset`
 * - `StageRuntime`
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Point-in-time concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note
 * The concept is stage-aware and performs different actions depending
 * on the encoding stage.
 */
template <std::size_t Stage, std::size_t Section, PointInTimeType Variant>
constexpr bool pointInTimeApplicable() {
    bool condition1 = Stage == StageAllocate && Section == SecProductDefinitionSection;
    bool condition2 = Stage == StagePreset && Section == SecProductDefinitionSection;
    bool condition3 = Stage == StageRuntime && Section == SecProductDefinitionSection;

    return (condition1 || condition2 || condition3);
}


/**
 * @brief Execute the `pointInTime` concept operation.
 *
 * This function implements the runtime logic of the GRIB `pointInTime` concept.
 * When applicable, it:
 *
 * - Deduces the forecast time offset from the input dictionaries
 * - Validates that the offset is an integer number of hours
 * - Encodes time-related GRIB keys according to the current encoding stage
 *
 * Stage-specific behavior:
 * - **StageAllocate**
 *   - Marks cutoff-related fields as missing
 * - **StagePreset**
 *   - Sets the time unit to hours
 * - **StageRuntime**
 *   - Sets the forecast time value in hours
 *
 * If the concept is invoked when not applicable, or if unsupported
 * forecast offsets are encountered, a `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Point-in-time concept variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary (currently unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @param[in]  mars MARS input dictionary
 * @param[in]  geo  Geometry dictionary (currently unused)
 * @param[in]  par  Parameter dictionary
 * @param[in]  opt  Options dictionary
 * @param[out] out  Output GRIB dictionary to be populated
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
 *         If:
 *         - the concept is called when not applicable
 *         - the forecast time is not an integer number of hours
 *         - any deduction or encoding step fails
 *
 * @note
 * - Currently, only full-hour forecast steps are supported.
 * - Sub-hour resolutions must be handled by a different concept
 *   or by extending this implementation.
 *
 * @see pointInTimeApplicable
 */
template <std::size_t Stage, std::size_t Section, PointInTimeType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void PointInTimeOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                   OutDict_t& out) {

    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (pointInTimeApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(pointInTime);

            // Deductions
            long marsStepInSeconds = deductions::resolve_ForecastTimeInSeconds_or_throw(mars, par, opt);

            // Basic checks
            if (marsStepInSeconds % 3600 != 0) {
                throw Mars2GribConceptException(
                    std::string(pointInTimeName), std::string(pointInTimeTypeName<Variant>()), std::to_string(Stage),
                    std::to_string(Section), "Only full hour steps are supported currently", Here());
            }
            long marsStepInHours = marsStepInSeconds / 3600;

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Stage == StageAllocate) {

                // Encoding
                setMissing_or_throw(out, "hoursAfterDataCutoff");
                setMissing_or_throw(out, "minutesAfterDataCutoff");
            }

            if constexpr (Stage == StagePreset) {

                // Encoding
                set_or_throw<long>(out, "indicatorOfUnitOfTimeRange", static_cast<long>(TimeUnit::Hour));
            }

            if constexpr (Stage == StageRuntime) {

                // Encoding
                set_or_throw<long>(out, "forecastTime", marsStepInHours);
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(pointInTime, "Unable to set `pointInTime` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(pointInTime, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

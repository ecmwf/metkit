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
/// @file shapeOfTheEarthOp.h
/// @brief Implementation of the GRIB `shapeOfTheEarth` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **shapeOfTheEarth concept** within the mars2grib backend.
///
/// The concept is responsible for populating the GRIB key `shapeOfTheEarth`
/// in the *Grid Definition Section* (Section 3), describing the geometric
/// reference system used for the Earth model (e.g. spherical Earth,
/// oblate spheroid, custom reference system).
///
/// The value is deduced from MARS and parameterization dictionaries and mapped
/// to the corresponding GRIB code table via a strongly-typed enumeration.
///
/// The implementation follows the standard mars2grib concept pattern:
/// - Compile-time applicability via `shapeOfTheEarthApplicable`
/// - Centralized deduction logic
/// - Strict GRIB-level encoding
/// - Context-rich error handling
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of `concepts`
/// to avoid conflicts with the C++20 `concepts` language feature.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarthEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/shapeOfTheEarth.h"

// Tables
#include "metkit/mars2grib/backend/tables/shapeOfTheReferenceSystem.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `shapeOfTheEarth` concept.
///
/// This predicate determines whether the `shapeOfTheEarth` concept
/// is applicable for a given encoding stage, GRIB section, and concept variant.
///
/// The concept is applied:
/// - exclusively in the *Grid Definition Section* (Section 3)
/// - during the preset stage
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Shape-of-the-Earth concept variant
///
/// @return `true` if the concept is applicable, `false` otherwise.
///
template <std::size_t Stage, std::size_t Section, ShapeOfTheEarthType Variant>
constexpr bool shapeOfTheEarthApplicable() {
    return Section == SecGridDefinitionSection && Stage == StagePreset;
}


///
/// @brief Execute the `shapeOfTheEarth` concept operation.
///
/// This function implements the runtime logic of the GRIB
/// `shapeOfTheEarth` concept.
///
/// When applicable, it:
/// 1. Deduces the Earth reference system from MARS, and parametrization dictionaries.
/// 2. Maps the result to a GRIB-compliant
/// `ShapeOfTheReferenceSystem` enumeration.
/// 3. Encodes the corresponding numeric value into the output GRIB
/// dictionary.
///
/// If the concept is invoked outside its applicability domain,
/// a `Mars2GribConceptException` is thrown.
///
/// @tparam Stage     Encoding stage (compile-time constant)
/// @tparam Section   GRIB section index
/// @tparam Variant   Shape-of-the-Earth concept variant
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
/// - the concept is called when not applicable
/// - deduction of the Earth shape fails
/// - encoding into the GRIB dictionary fails
///
/// @note
/// - This concept performs a **pure GRIB-level encoding**.
/// - All semantic interpretation of geometry is delegated to the
/// corresponding deduction.
///
/// @see shapeOfTheEarthApplicable
/// @see deductions::resolve_ShapeOfTheEarth_or_throw
///
template <std::size_t Stage, std::size_t Section, ShapeOfTheEarthType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ShapeOfTheEarthOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (shapeOfTheEarthApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(shapeOfTheEarth);

            // Deductions
            tables::ShapeOfTheReferenceSystem shapeOfTheEarth =
                deductions::resolve_ShapeOfTheEarth_or_throw(mars, par, opt);

            // Encoding
            set_or_throw<long>(out, "shapeOfTheEarth", static_cast<long>(shapeOfTheEarth));
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(shapeOfTheEarth, "Unable to set `shapeOfTheEarth` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(shapeOfTheEarth, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

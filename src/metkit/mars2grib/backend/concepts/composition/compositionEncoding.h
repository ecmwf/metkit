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
/// @file compositionOp.h
/// @brief Implementation of the GRIB `composition` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **composition concept** within the mars2grib backend.
///
/// The concept is responsible for populating GRIB keys related to the
/// *composition* of the encoded product, specifically the
/// `constituentType` key in the Product Definition Section.
///
/// The composition concept is variant-driven. Different variants correspond
/// to different composition semantics (e.g. chemical constituents), and
/// only selected variants perform encoding actions.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `compositionApplicable`
/// - Variant-specific runtime deduction
/// - Strict error handling with contextual concept exceptions
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of `concepts`
/// to avoid ambiguity and potential conflicts with the C++20 `concept` language
/// feature and related standard headers.
///
/// This is a deliberate design choice and must not be changed.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/composition/compositionEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/constituentType.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `composition` concept.
///
/// This predicate determines whether the `composition` concept is applicable
/// for a given combination of:
/// - encoding stage
/// - GRIB section
/// - concept variant
///
/// Applicability is evaluated entirely at compile time and is used by the
/// concept dispatcher to ensure that only valid concept instantiations occur.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Composition concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
/// @note
/// The default applicability rule enables the concept only when:
/// - `Stage == StagePreset`
/// - `Section == SecProductDefinitionSection`
///
/// Variant-specific behavior is handled inside the concept operation.
///
template <std::size_t Stage, std::size_t Section, CompositionType Variant>
constexpr bool compositionApplicable() {
    return (Stage == StagePreset && Section == SecProductDefinitionSection);
}


///
/// @brief Execute the `composition` concept operation.
///
/// This function implements the runtime logic of the GRIB `composition` concept.
/// When applicable, it:
///
/// - Performs variant-specific deductions from the MARS and parameter dictionaries
/// - Encodes the corresponding GRIB keys into the output dictionary
///
/// Currently, only the `CompositionType::Chem` variant performs encoding,
/// setting the GRIB key `constituentType`.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Composition concept variant
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
/// - a deduction fails
/// - a GRIB key cannot be set
///
/// @note
/// - All runtime errors are wrapped with full concept context
/// (concept name, variant, stage, section).
/// - Variants not explicitly handled result in a no-op when applicable.
///
/// @see compositionApplicable
///
template <std::size_t Stage, std::size_t Section, CompositionType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void CompositionOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (compositionApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(composition);

            // =============================================================
            // Structural validation
            // =============================================================
            /// @todo [owner: dgov][scope: concept][reason: completeness][prio: low]

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Variant == CompositionType::Chem) {

                // Structural validation
                /// @todo [owner: dgov][scope: concept][reason: completeness][prio: low]

                // Deductions
                long constituentType = deductions::resolve_ConstituentType_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "constituentType", constituentType);
            }
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(composition, "Unable to set `composition` concept...");
        }

        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(composition, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

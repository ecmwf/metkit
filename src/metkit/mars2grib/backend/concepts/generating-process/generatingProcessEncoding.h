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
 * @file generatingProcessOp.h
 * @brief Implementation of the GRIB `generatingProcess` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **generatingProcess concept** within the mars2grib backend.
 *
 * The generatingProcess concept is responsible for populating GRIB keys
 * related to the *origin and nature of the data generation process*, including:
 *
 * - `backgroundProcess`
 * - `generatingProcessIdentifier`
 * - `typeOfGeneratingProcess`
 *
 * These keys are encoded in the Product Definition Section and are tightly
 * coupled to both MARS semantics and GRIB code tables.
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `generatingProcessApplicable`
 * - Delegation of semantic resolution to dedicated deduction functions
 * - Explicit handling of legacy encoder behavior
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
#include "metkit/mars2grib/backend/concepts/generating-process/generatingProcessEnum.h"

// Tables
#include "metkit/mars2grib/backend/tables/backgroundProcess.h"
#include "metkit/mars2grib/backend/tables/typeOfGeneratingProcess.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/backgroundProcess.h"
#include "metkit/mars2grib/backend/deductions/generatingProcessIdentifier.h"
#include "metkit/mars2grib/backend/deductions/typeOfGeneratingProcess.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `generatingProcess` concept.
 *
 * This predicate determines whether the generatingProcess concept is applicable
 * for a given combination of:
 * - encoding stage
 * - GRIB section
 * - concept variant
 *
 * Applicability is evaluated entirely at compile time and is used by the
 * concept dispatcher to ensure that only valid concept instantiations occur.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Generating process concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note
 * The default applicability rule enables the concept when:
 * - the encoding stage is `StagePreset`, **or**
 * - the concept variant is `GeneratingProcessType::Default`
 * - and the GRIB section is `SecIdentificationSection`
 *
 * This permissive rule reflects the historical behavior of the encoder and
 * allows the concept to participate in multiple encoding paths.
 */
template <std::size_t Stage, std::size_t Section, GeneratingProcessType Variant>
constexpr bool generatingProcessApplicable() {
    return ((Section == SecProductDefinitionSection) &&
            (Stage == StagePreset) &&
            (Variant == GeneratingProcessType::Default));
}


/**
 * @brief Execute the generatingProcess concept operation.
 *
 * This function implements the runtime logic of the GRIB `generatingProcess`
 * concept. When applicable, it:
 *
 * 1. Resolves the background process associated with the data.
 * 2. Optionally resolves the generating process identifier.
 * 3. Optionally resolves the type of generating process.
 *
 * The current implementation contains **explicit legacy compatibility paths**
 * that mirror the behavior of the previous encoder, including reliance on
 * pre-existing GRIB header state and ecCodes side effects.
 *
 * These paths are clearly marked and must be removed in a future cleanup
 * iteration.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Generating process concept variant
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
 *         - any deduction fails
 *         - a required GRIB key cannot be set
 *         - the concept is invoked outside its applicability domain
 *
 * @note
 * - All runtime errors are wrapped with full concept context
 *   (concept name, variant, stage, section).
 * - Several code paths are intentionally marked as legacy and must not
 *   be extended.
 *
 * @todo [owner: mds,dgov][scope: concept][reason: legacy][prio: high]
 * - Remove all reliance on `std::optional` forwarding.
 * - Remove reliance on ecCodes implicit behavior.
 * - Enforce explicit defaults, mandatory values, or hard failures.
 *
 * @see generatingProcessApplicable
 */
template <std::size_t Stage, std::size_t Section, GeneratingProcessType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void GeneratingProcessOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                         OutDict_t& out) {

    using metkit::mars2grib::backend::tables::TypeOfGeneratingProcess;
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (generatingProcessApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(generatingProcess);

            // Retrieve the information
            std::optional<long> generatingProcessIdentifier =
                deductions::resolve_GeneratingProcessIdentifier_opt(mars, par, opt);
            std::optional<TypeOfGeneratingProcess> typeOfGeneratingProcess =
                deductions::resolve_TypeOfGeneratingProcess_opt(mars, par, opt);


            /// @todo [owner: mds,dgov][scope: concept][reason: legacy][prio: high]
            /// Remove this logic.
            ///
            /// This is a legacy artifact of the previous encoder implementation and
            /// relies on ecCodes implicitly setting `backgroundProcess` from
            /// `mars::model`.
            if (bool useModel = get_opt<bool>(opt, "useModelForBackgroundProcess").value_or(true); useModel) {
                std::string modelVal = get_or_throw<std::string>(mars, "model");
                set_or_throw<std::string>(out, "model", modelVal);
            }
            else {
                tables::BackgroundProcess backgroundProcess =
                    deductions::resolve_BackgroundProcess_or_throw(mars, par, opt);
                set_or_throw<long>(out, "backgroundProcess", static_cast<long>(backgroundProcess));
            }

            /// @todo [owner: mds,dgov][scope: concept][reason: legacy][prio: high]
            /// Remove this logic.
            ///
            /// Deductions must not forward `std::optional` values directly.
            /// A proper deduction must:
            ///   - set an explicit value (e.g. `Missing`),
            ///   - apply a DGOV-approved default,
            ///   - or throw if the value is mandatory.
            if (generatingProcessIdentifier.has_value()) {
                set_or_throw<long>(out, "generatingProcessIdentifier", generatingProcessIdentifier.value());
            }

            /// @todo [owner: mds,dgov][scope: concept][reason: legacy][prio: high]
            /// Remove this logic.
            ///
            /// Relying on pre-existing GRIB header values is not reproducible
            /// and must be eliminated.
            if (typeOfGeneratingProcess.has_value()) {
                set_or_throw<long>(out, "typeOfGeneratingProcess", static_cast<long>(typeOfGeneratingProcess.value()));
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(generatingProcess, "Unable to set `generatingProcess` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(generatingProcess, "Concept called when not applicable...");


    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

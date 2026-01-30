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
 * @file ensembleOp.h
 * @brief Implementation of the GRIB `ensemble` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **ensemble concept** within the mars2grib backend.
 *
 * The ensemble concept is responsible for encoding GRIB keys related to
 * ensemble forecasts in the *Product Definition Section*, including:
 *
 * - `typeOfEnsembleForecast`
 * - `numberOfForecastsInEnsemble`
 * - `perturbationNumber`
 *
 * The concept currently supports the **Individual ensemble variant**, which
 * represents a single ensemble member within an ensemble forecast system.
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `ensembleApplicable`
 * - Structural validation of the Product Definition Section
 * - Delegation of semantic resolution to dedicated deduction functions
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
#include "metkit/mars2grib/backend/concepts/ensemble/ensembleEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/numberOfForecastsInEnsemble.h"
#include "metkit/mars2grib/backend/deductions/perturbationNumber.h"
#include "metkit/mars2grib/backend/deductions/typeOfEnsembleForecast.h"

// Tables
#include "metkit/mars2grib/backend/tables/typeOfEnsembleForecast.h"

// checks
#include "metkit/mars2grib/backend/checks/checkEnsembleProductDefinitionSection.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `ensemble` concept.
 *
 * This predicate determines whether the ensemble concept is applicable
 * for a given combination of:
 * - encoding stage
 * - GRIB section
 * - ensemble variant
 *
 * Applicability is evaluated entirely at compile time and is used by the
 * concept dispatcher to ensure that only valid concept instantiations occur.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Ensemble concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note
 * The default applicability rule enables the concept only when:
 * - `Variant == EnsembleType::Individual`
 * - `Stage == StagePreset`
 * - `Section == SecProductDefinitionSection`
 *
 * This reflects the current GRIB encoding rules for individual ensemble members.
 */
template <std::size_t Stage, std::size_t Section, EnsembleType Variant>
constexpr bool ensembleApplicable() {
    // Confitions to apply concept
    return ((Variant == EnsembleType::Individual) && (Stage == StagePreset) &&
            (Section == SecProductDefinitionSection));
}


/**
 * @brief Execute the ensemble concept operation.
 *
 * This function implements the runtime logic of the GRIB `ensemble` concept.
 * When applicable, it:
 *
 * 1. Validates that the Product Definition Section is compatible with
 *    ensemble encoding.
 * 2. Deduces ensemble-related metadata from MARS and parameter dictionaries.
 * 3. Encodes the corresponding GRIB keys in the output dictionary.
 *
 * The concept currently supports the **Individual** ensemble variant, which
 * represents a single ensemble member.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Ensemble concept variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @param[in]  mars MARS input dictionary
 * @param[in]  par  Parameter dictionary
 * @param[in]  opt  Options dictionary
 * @param[out] out  Output GRIB dictionary to be populated
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
 *         If:
 *         - the Product Definition Section is not compatible with ensemble encoding
 *         - any deduction fails
 *         - any GRIB key cannot be set
 *
 * @note
 * - All runtime errors are wrapped with full concept context
 *   (concept name, variant, stage, section).
 * - This concept does not rely on any pre-existing GRIB header state.
 *
 * @see ensembleApplicable
 */
template <std::size_t Stage, std::size_t Section, EnsembleType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void EnsembleOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (ensembleApplicable<Stage, Section, Variant>()) {

        try {

            // Logging
            MARS2GRIB_LOG_CONCEPT(ensemble);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Variant == EnsembleType::Individual) {

                // Structural validation
                validation::check_EnsembleProductDefinitionSection_or_throw(opt, out);

                // Deductions
                tables::TypeOfEnsembleForecast typeOfEnsembleForecast =
                    deductions::resolve_TypeOfEnsembleForecast_or_throw(mars, par, opt);
                long numberOfForecastsInEnsemble =
                    deductions::resolve_NumberOfForecastsInEnsemble_or_throw(mars, par, opt);
                long marsNumber = deductions::resolve_PerturbationNumber_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "typeOfEnsembleForecast", static_cast<long>(typeOfEnsembleForecast));
                set_or_throw<long>(out, "numberOfForecastsInEnsemble", numberOfForecastsInEnsemble);
                set_or_throw<long>(out, "perturbationNumber", marsNumber);
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(ensemble, "Unable to set `ensemble` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(ensemble, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

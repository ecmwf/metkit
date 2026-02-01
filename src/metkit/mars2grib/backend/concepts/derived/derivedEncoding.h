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
 * @file derivedOp.h
 * @brief Implementation of the GRIB `derived` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **derived concept** within the mars2grib backend.
 *
 * The concept is responsible for populating GRIB keys related to
 * *derived ensemble products*, including:
 *
 * - `derivedForecast`
 * - `numberOfForecastsInEnsemble`
 *
 * These keys are encoded in the Product Definition Section and are used
 * to describe ensemble-derived statistical products (e.g. means, spreads,
 * probabilities).
 *
 * The concept implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `derivedApplicable`
 * - Runtime structural validation of the Product Definition Section
 * - Delegation of semantic deduction to dedicated deduction functions
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
#include "metkit/mars2grib/backend/concepts/derived/derivedEnum.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/derivedForecast.h"
#include "metkit/mars2grib/backend/deductions/numberOfForecastsInEnsemble.h"

// Tables
#include "metkit/mars2grib/backend/tables/derivedForecast.h"

// Checks
#include "metkit/mars2grib/backend/checks/checkDerivedProductDefinitionSection.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `derived` concept.
 *
 * This predicate determines whether the `derived` concept is applicable
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
 * @tparam Variant Derived concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note The default applicability rule enables the concept only when:
 * - `Stage == StagePreset`
 * - `Section == SecProductDefinitionSection`
 *
 * @todo [owner: mds][scope: concept][reason: correctness][prio: medium]
 * - Refine applicability rules once derived product usage is fully constrained
 *   by stage, section, or variant.
 */
template <std::size_t Stage, std::size_t Section, DerivedType Variant>
constexpr bool derivedApplicable() {
    return (Stage == StagePreset) && (Section == SecProductDefinitionSection);
}

/**
 * @brief Execute the `derived` concept operation.
 *
 * This function implements the runtime logic of the GRIB `derived` concept.
 * When applicable, it:
 *
 * 1. Validates that the Product Definition Section is compatible with
 *    derived ensemble products.
 * 2. Deduces the type of derived forecast from MARS and parameter dictionaries.
 * 3. Deduces the number of ensemble members involved.
 * 4. Encodes the corresponding GRIB keys in the output dictionary.
 *
 * The concept acts as a **coordination layer**:
 * - Structural validation is performed explicitly.
 * - Semantic deduction is delegated to backend deductions.
 * - Value correctness is guaranteed by GRIB table-backed enumerations.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Derived concept variant
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
 *         - structural validation of the Product Definition Section fails
 *         - a deduction fails
 *         - a GRIB key cannot be set
 *
 * @note
 * - All runtime errors are wrapped with full concept context
 *   (concept name, variant, stage, section).
 * - This concept does not rely on any pre-existing GRIB header state.
 *
 * @see derivedApplicable
 */
template <std::size_t Stage, std::size_t Section, DerivedType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void DerivedOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (derivedApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(derived);

            // Structural validation
            validation::check_DerivedProductDefinitionSection_or_throw(opt, out);

            // Deductions
            tables::DerivedForecast derivedForecast = deductions::resolve_DerivedForecast_or_throw(mars, par, opt);
            long numberOfForecastsInEnsemble = deductions::resolve_NumberOfForecastsInEnsemble_or_throw(mars, par, opt);

            // Encoding
            set_or_throw<long>(out, "derivedForecast", static_cast<long>(derivedForecast));
            set_or_throw<long>(out, "numberOfForecastsInEnsemble", numberOfForecastsInEnsemble);
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(derived, "Unable to set `derived` concept...");
        }

        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(derived, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

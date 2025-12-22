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
 * @file analysisOp.h
 * @brief Implementation of the GRIB `analysis` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **analysis concept** within the mars2grib backend.
 *
 * The concept is responsible for populating GRIB keys related to the
 * *Local Use Section* analysis metadata, based on information extracted
 * from MARS input dictionaries and validated against GRIB constraints.
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `analysisApplicable`
 * - Runtime validation and deduction
 * - Strict error handling with contextual concept exceptions
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of `concepts`
 * to avoid ambiguity and potential conflicts with the C++20 `concept` language
 * feature and related standard headers.
 *
 * This is a deliberate design choice and must not be changed.

 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/analysis/analysisEnum.h"
#include "metkit/mars2grib/backend/concepts/conceptCore.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/lengthOfTimeWindow.h"
#include "metkit/mars2grib/backend/deductions/offsetToEndOf4DvarWindow.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchLocalDefinitionNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `analysis` concept.
 *
 * This function determines whether the `analysis` concept is applicable
 * for a given combination of:
 * - encoding stage
 * - GRIB section
 * - concept variant
 *
 * The applicability is evaluated entirely at compile time and is used
 * by the concept dispatcher to ensure that only valid concept invocations
 * are instantiated.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Analysis concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note
 * The default applicability rule enables the concept only when:
 * - `Variant == AnalysisType::Default`
 * - `Stage == StagePreset`
 * - `Section == SecLocalUseSection`
 *
 * Users may override or specialize this predicate to alter applicability.
 */
template <std::size_t Stage, std::size_t Section, AnalysisType Variant>
constexpr bool analysisApplicable() {

    // Conditions to apply concept
    return ((Variant == AnalysisType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}


/**
 * @brief Execute the `analysis` concept operation.
 *
 * This function implements the runtime logic of the GRIB `analysis` concept.
 * When applicable, it:
 *
 * 1. Verifies GRIB preconditions for the Local Use Section.
 * 2. Deduces required analysis-related values from MARS and parameter dictionaries.
 * 3. Encodes the corresponding GRIB keys in the output dictionary.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage    Encoding stage (compile-time constant)
 * @tparam Section  GRIB section index (compile-time constant)
 * @tparam Variant  Analysis concept variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary
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
 *         - required GRIB preconditions are not satisfied
 *         - any deduction or encoding step fails
 *
 * @note
 * - All runtime errors are wrapped with full concept context
 *   (concept name, variant, stage, section).
 * - This function does not rely on any pre-existing GRIB header state.
 *
 * @see analysisApplicable
 */
template <std::size_t Stage, std::size_t Section, AnalysisType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void AnalysisOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (analysisApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(analysis);

            // Structural validation
            validation::match_LocalDefinitionNumber_or_throw(opt, out, {36L});

            // Deductions
            auto offsetToEndOf4DvarWindowVal = deductions::resolve_offsetToEndOf4DvarWindow_or_throw(mars, par, opt);
            long lengthOfTimeWindowVal       = deductions::resolve_LengthOfTimeWindowInSeconds_or_throw(mars, par, opt);

            // Encoding
            set_or_throw<long>(out, "offsetToEndOf4DvarWindow", offsetToEndOf4DvarWindowVal);
            set_or_throw<long>(out, "lengthOf4DvarWindow", lengthOfTimeWindowVal / 3600);
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(analysis, "Unable to set `analysis` concept...");
        }

        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(analysis, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

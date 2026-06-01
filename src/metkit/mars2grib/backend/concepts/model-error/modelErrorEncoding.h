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
/// @file modelErrorOp.h
/// @brief Implementation of the GRIB `modelError` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **modelError concept** within the mars2grib backend.
///
/// The modelError concept is responsible for encoding GRIB keys related to
/// *model-error metadata* stored in the Local Use Section, specifically:
///
/// - `componentIndex`
/// - `numberOfComponents`
/// - `modelErrorType`
///
/// These fields identify the realization within a model-error ensemble,
/// the total number of realizations, and the type of model error.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `modelErrorApplicable`
/// - Runtime validation of Local Definition Number
/// - Explicit deduction of required values
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

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/model-error/modelErrorEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/componentIndex.h"
#include "metkit/mars2grib/backend/deductions/modelErrorType.h"
#include "metkit/mars2grib/backend/deductions/numberOfComponents.h"

// checks
#include "metkit/mars2grib/backend/checks/matchLocalDefinitionNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `modelError` concept.
///
/// This predicate determines whether the modelError concept is applicable
/// for a given combination of:
/// - encoding stage
/// - GRIB section
/// - concept variant
///
/// Applicability is evaluated entirely at compile time and is used by the
/// concept dispatcher to control instantiation and execution.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant ModelError concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
/// @note
/// The default applicability rule enables the concept only when:
/// - `Variant == ModelErrorType::Default`
/// - `Stage == StagePreset`
/// - `Section == SecLocalUseSection`
///
template <std::size_t Stage, std::size_t Section, ModelErrorType Variant>
constexpr bool modelErrorApplicable() {
    return ( (Stage == StagePreset) && (Section == SecLocalUseSection) );
}


///
/// @brief Execute the `modelError` concept operation.
///
/// This function implements the runtime logic of the GRIB `modelError` concept.
/// When applicable, it:
///
/// 1. Validates that the Local Use Section matches the expected definition.
/// 2. Deduces the model-error related identifiers.
/// 3. Encodes the corresponding GRIB keys in the output dictionary.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    ModelError concept variant
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
/// - the Local Definition Number does not match expectations,
/// - required deductions fail,
/// - any GRIB key cannot be set,
/// - the concept is invoked when not applicable.
///
/// @note
/// - All runtime errors are wrapped with full concept context
/// (concept name, variant, stage, section).
/// - This concept does not rely on pre-existing GRIB header state.
///
/// @see modelErrorApplicable
///
template <std::size_t Stage, std::size_t Section, ModelErrorType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ModelErrorOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (modelErrorApplicable<Stage, Section, Variant>()) {


        try {

            MARS2GRIB_LOG_CONCEPT(modelError);

            if ( Variant == ModelErrorType::ComponentIndex ) {
                validation::match_LocalDefinitionNumber_or_throw(opt, out, {25L, 39L});

                // Deductions
                auto componentIndexVal     = deductions::resolve_ComponentIndex_or_throw(mars, par, opt);
                auto numberOfComponentsVal = deductions::resolve_NumberOfComponents_or_throw(mars, par, opt);
                auto modelErrorTypeVal     = deductions::resolve_ModelErrorType_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "componentIndex", componentIndexVal);
                set_or_throw<long>(out, "numberOfComponents", numberOfComponentsVal);
                set_or_throw<long>(out, "modelErrorType", modelErrorTypeVal);


            }
            else if ( Variant == ModelErrorType::FourierCoefficients ) {
                validation::match_LocalDefinitionNumber_or_throw(opt, out, {45L});

                MARS2GRIB_CONCEPT_THROW(modelError, "Variant not implemented...");

            }
            else {
                MARS2GRIB_CONCEPT_THROW(modelError, "Unknown variant...");
            }

        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(modelError, "Unable to set `modelError` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(modelError, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

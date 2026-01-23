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
 * @file marsOp.h
 * @brief Implementation of the GRIB `mars` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **mars concept** within the mars2grib backend.
 *
 * The mars concept is responsible for encoding core MARS identity metadata
 * into the GRIB *Local Use Section*, specifically:
 *
 * - `class`
 * - `type`
 * - `stream`
 * - `expver`
 *
 * These fields collectively define the provenance and classification of the
 * encoded product and are required by downstream systems and workflows.
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `marsApplicable`
 * - Runtime structural validation of the Local Use Section
 * - Explicit deduction of all required MARS identity fields
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
#include "metkit/mars2grib/backend/concepts/mars/marsEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/class.h"
#include "metkit/mars2grib/backend/deductions/expver.h"
#include "metkit/mars2grib/backend/deductions/stream.h"
#include "metkit/mars2grib/backend/deductions/type.h"

// checks
#include "metkit/mars2grib/backend/checks/checkLocalUseSection.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `mars` concept.
 *
 * This predicate determines whether the mars concept is applicable
 * for a given combination of:
 * - encoding stage
 * - GRIB section
 * - concept variant
 *
 * Applicability is evaluated entirely at compile time and is used by the
 * concept dispatcher to ensure that only valid concept invocations
 * are instantiated.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Mars concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note
 * The default applicability rule enables the concept only when:
 * - `Variant == MarsType::Default`
 * - `Stage == StagePreset`
 * - `Section == SecLocalUseSection`
 */
template <std::size_t Stage, std::size_t Section, MarsType Variant>
constexpr bool marsApplicable() {

    // Confitions to apply concept
    return ((Variant == MarsType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}


/**
 * @brief Execute the `mars` concept operation.
 *
 * This function implements the runtime logic of the GRIB `mars` concept.
 * When applicable, it:
 *
 * 1. Validates the structural integrity of the GRIB Local Use Section.
 * 2. Deduces core MARS identity fields from the input dictionaries.
 * 3. Encodes the corresponding GRIB keys in the output dictionary.
 *
 * The concept establishes the fundamental identity of the GRIB message
 * and is typically a prerequisite for other Local Use Section concepts.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Mars concept variant
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
 *         - the Local Use Section is structurally invalid,
 *         - any required MARS field cannot be deduced,
 *         - any GRIB key cannot be set,
 *         - the concept is invoked when not applicable.
 *
 * @note
 * - All runtime errors are wrapped with full concept context
 *   (concept name, variant, stage, section).
 * - This concept does not rely on pre-existing GRIB header state.
 *
 * @note
 * The keywords [marsClass, marsType, marsStream] correspond to *raw GRIB keys*
 * and are written directly without triggering additional logic.
 *
 * In contrast, the high-level keywords [class, type, stream] are **ecCodes
 * concepts**. Setting them may implicitly modify multiple underlying GRIB
 * keys in order to maintain internal consistency.
 *
 * As a consequence, assigning high-level keywords can have side effects.
 * Examples (non-exhaustive) include:
 *   - setting "type" may implicitly update "typeOfProcessedData"
 *   - setting "stream" may implicitly change the product definition template number
 *
 * @see marsApplicable
 */
template <std::size_t Stage, std::size_t Section, MarsType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void MarsOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    // eccodes/definitions/grib2/local.98.36.def
    if constexpr (marsApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(mars);

            // Preconditions/contracts
            validation::check_LocalUseSection_or_throw(opt, out);


            // Deductions
            std::string marsClassVal  = deductions::resolve_Class_or_throw(mars, par, opt);
            std::string marsTypeVal   = deductions::resolve_Type_or_throw(mars, par, opt);
            std::string marsStreamVal = deductions::resolve_Stream_or_throw(mars, par, opt);
            std::string marsExpverVal = deductions::resolve_Expver_or_throw(mars, par, opt);

            // Encoding
            if (bool enableSideEffects = get_opt<bool>(opt, "enableSideEffects").value_or(false); enableSideEffects) {
                // Enabling this can lead to very hard-to-track side effects and/or failures of the whole encoding
                // chain, because setting high-level keys may implicitly modify multiple underlying GRIB keys.
                // Use with extreme caution and only when you fully understand the implications.
                set_or_throw<std::string>(out, "class", marsClassVal);
                set_or_throw<std::string>(out, "type", marsTypeVal);
                set_or_throw<std::string>(out, "stream", marsStreamVal);
            }
            else {
                set_or_throw<std::string>(out, "marsClass", marsClassVal);
                set_or_throw<std::string>(out, "marsType", marsTypeVal);
                set_or_throw<std::string>(out, "marsStream", marsStreamVal);
            }
            set_or_throw<std::string>(out, "expver", marsExpverVal);
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(mars, "Unable to set `mars` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(mars, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

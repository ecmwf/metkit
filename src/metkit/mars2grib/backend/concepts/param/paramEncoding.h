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
 * @file paramOp.h
 * @brief Implementation of the GRIB `param` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **param concept** within the mars2grib backend.
 *
 * The param concept is responsible for resolving and encoding the GRIB
 * parameter identifier (`paramId`) in the *Product Definition Section*.
 * The value is deduced from the MARS and parameter dictionaries using
 * dedicated deduction logic.
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `paramApplicable`
 * - Runtime deduction of the parameter identifier
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
#include "metkit/mars2grib/backend/concepts/param/paramEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/paramId.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `param` concept.
 *
 * The default applicability enables this concept only when:
 * - `Variant == ParamType::ParamId`
 * - `Stage == StagePreset` or `Stage == StageRuntime`
 * - `Section == SecProductDefinitionSection`
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Parameter concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 *
 * @note
 * The concept is intentionally enabled both at preset and runtime stages
 * to allow late binding of the parameter identifier if required.
 */
template <std::size_t Stage, std::size_t Section, ParamType Variant>
constexpr bool paramApplicable() {

    // Confitions to apply concept
    return ((Variant == ParamType::ParamId) && (Stage == StagePreset || Stage == StageRuntime) &&
            (Section == SecProductDefinitionSection));
}


/**
 * @brief Execute the `param` concept operation.
 *
 * This function implements the runtime logic of the GRIB `param` concept.
 * When applicable, it:
 *
 * 1. Deduces the GRIB parameter identifier (`paramId`) from the input
 *    MARS and parameter dictionaries.
 * 2. Encodes the resolved `paramId` into the GRIB output dictionary.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Parameter concept variant
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
 *         - the parameter identifier cannot be resolved
 *         - any encoding step fails
 *
 * @note
 * - This concept performs no implicit defaulting.
 * - The resolved `paramId` is expected to be fully validated by the
 *   underlying deduction logic.
 *
 * @see paramApplicable
 */
template <std::size_t Stage, std::size_t Section, ParamType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ParamOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (paramApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(param);

            // Deductions
            long paramId = deductions::resolve_ParamId_or_throw(mars, par, opt);

            // Encoding
            set_or_throw<long>(out, "paramId", paramId);
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(param, "Unable to set `param` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(param, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

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
 * @file originOp.h
 * @brief Implementation of the GRIB `origin` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **origin concept** within the mars2grib backend.
 *
 * The origin concept is responsible for encoding GRIB metadata identifying
 * the producing centre and sub-centre. In the current backend this is written
 * into the Local Use Section using:
 * - `origin`   (string centre identifier)
 * - `subCentre` (numeric sub-centre identifier)
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `originApplicable`
 * - Runtime deduction
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

// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/origin/originEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/centre.h"
#include "metkit/mars2grib/backend/deductions/subCentre.h"


// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `origin` concept.
 *
 * The default applicability enables this concept only when:
 * - `Variant == OriginType::Default`
 * - `Stage == StagePreset`
 * - `Section == SecLocalUseSection`
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Origin concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, OriginType Variant>
constexpr bool originApplicable() {

    // Conditions to apply concept
    return ((Variant == OriginType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}

/**
 * @brief Execute the `origin` concept operation.
 *
 * When applicable, this concept:
 * 1. Deduces the producing centre (`origin`) from the MARS dictionary.
 * 2. Deduces the numeric sub-centre (`subCentre`) from the parameter dictionary,
 *    defaulting to `0` when not provided (as implemented by the deduction).
 * 3. Encodes both keys into the output GRIB dictionary.
 *
 * If invoked when not applicable, a `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Origin concept variant
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
 *         - any deduction or encoding step fails
 *
 * @note
 * This concept does not rely on any pre-existing GRIB header state.
 *
 * @see originApplicable
 * @see metkit::mars2grib::backend::deductions::resolve_Centre_or_throw
 * @see metkit::mars2grib::backend::deductions::resolve_SubCentre_or_throw
 */
template <std::size_t Stage, std::size_t Section, OriginType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void OriginOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
              OutDict_t& out) noexcept(false) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (originApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(origin);

            // Deductions
            std::string centre = deductions::resolve_Centre_or_throw(mars, par, opt);
            long subCentre     = deductions::resolve_SubCentre_or_throw(mars, par, opt);

            // Encoding
            set_or_throw<std::string>(out, "origin", centre);
            set_or_throw<long>(out, "subCentre", subCentre);
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(origin, "Unable to set `origin` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(origin, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

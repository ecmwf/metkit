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
/// @file referencePeriodEncoding.h
/// @brief Implementation of the GRIB `referencePeriod` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **referencePeriod concept** within the mars2grib backend.
///
/// The current implementation is an inactive skeleton that provides the
/// standard compile-time and runtime hooks required by the concept registry,
/// without enabling any encoding behavior yet.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `referencePeriodApplicable`
/// - Runtime execution via `ReferencePeriodOp`
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
#include "metkit/mars2grib/backend/concepts/reference-period/referencePeriodEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `referencePeriod` concept.
///
/// This predicate determines whether the `referencePeriod` concept is
/// applicable for a given combination of:
/// - encoding stage
/// - GRIB section
/// - concept variant
///
/// Applicability is evaluated entirely at compile time and is used by the
/// concept dispatcher to control instantiation and execution.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Reference-period concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
/// @note
/// The current skeleton implementation always returns `false`.
///
template <std::size_t Stage, std::size_t Section, ReferencePeriodType Variant>
constexpr bool referencePeriodApplicable() {
    return false;
}


///
/// @brief Execute the `referencePeriod` concept operation.
///
/// This function provides the standard runtime hook for the
/// `referencePeriod` concept.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Reference-period concept variant
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
/// If the concept is invoked when not applicable.
///
/// @see referencePeriodApplicable
///
template <std::size_t Stage, std::size_t Section, ReferencePeriodType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ReferencePeriodOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    static_cast<void>(mars);
    static_cast<void>(par);
    static_cast<void>(opt);
    static_cast<void>(out);

    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (referencePeriodApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(referencePeriod);
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(referencePeriod, "Unable to set `referencePeriod` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(referencePeriod, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

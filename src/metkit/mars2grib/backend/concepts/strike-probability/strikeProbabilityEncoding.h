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
/// @file strikeProbabilityEncoding.h
/// @brief Implementation of the GRIB `strikeProbability` concept operation.
///
/// This header defines the **strikeProbability concept** infrastructure within
/// the mars2grib backend.
///
/// The strikeProbability concept currently exists only as a registered semantic
/// placeholder:
/// - it defines the standard compile-time concept interface,
/// - it participates in the registry infrastructure,
/// - it is intentionally inactive until matching and encoding rules are defined.
///
/// This implementation therefore follows the conservative placeholder model:
/// - Compile-time applicability via `strikeProbabilityApplicable`
/// - No active encoding domain
/// - Explicit failure if the concept is invoked unexpectedly
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
#include "metkit/mars2grib/backend/concepts/strike-probability/strikeProbabilityEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `strikeProbability` concept.
///
/// The strikeProbability concept is intentionally inactive until its runtime
/// semantics are defined.
///
/// This predicate always evaluates to `false` and exists to satisfy the
/// uniform concept interface expected by the concept dispatcher.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Strike-probability concept variant
///
/// @return Always `false`.
///
template <std::size_t Stage, std::size_t Section, StrikeProbabilityType Variant>
constexpr bool strikeProbabilityApplicable() {
    return false;
}


///
/// @brief Execute the `strikeProbability` concept operation.
///
/// This function implements the registered runtime hook for the
/// `strikeProbability` concept.
///
/// Because the concept is currently a placeholder, it must never perform any
/// encoding and any invocation is treated as a programming error.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Strike-probability concept variant
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the GRIB output dictionary
///
/// @param[in]  mars MARS input dictionary (unused)
/// @param[in]  par  Parameter dictionary (unused)
/// @param[in]  opt  Options dictionary (unused)
/// @param[out] out  Output GRIB dictionary (unused)
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
/// Always thrown if this function is invoked.
///
/// @note
/// This function intentionally does not provide a silent no-op.
/// Any invocation is treated as an unexpected use of incomplete concept logic.
///
template <std::size_t Stage, std::size_t Section, StrikeProbabilityType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void StrikeProbabilityOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt,
                         OutDict_t& out) noexcept(false) {

    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (strikeProbabilityApplicable<Stage, Section, Variant>()) {

        // Debug output
        MARS2GRIB_LOG_CONCEPT(strikeProbability);

        // Successful no-op
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(strikeProbability, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

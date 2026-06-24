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
/// @file nilOp.h
/// @brief Implementation of the GRIB `nil` concept operation.
///
/// This header defines the **nil concept**, a sentinel / placeholder concept
/// used within the mars2grib backend.
///
/// The nil concept:
/// - Has no semantic meaning at the GRIB level
/// - Must never be applicable
/// - Must never modify the output GRIB dictionary
///
/// Its primary purposes are:
/// - Acting as a compile-time placeholder in concept tables
/// - Providing a well-defined failure mode if accidentally invoked
/// - Making concept dispatch logic total (no missing concept slots)
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
#include "metkit/mars2grib/backend/concepts/nil/nilEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `nil` concept.
///
/// The nil concept is **never applicable**.
///
/// This predicate always evaluates to `false` and exists only to satisfy
/// the uniform concept interface expected by the concept dispatcher.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Nil concept variant
///
/// @return Always `false`.
///
template <std::size_t Stage, std::size_t Section, NilType Variant>
constexpr bool nilApplicable() {
    return false;
}

///
/// @brief Execute the `nil` concept operation.
///
/// This function must never perform any operation.
///
/// If invoked when not applicable (which is always the case),
/// a `Mars2GribConceptException` is thrown with full contextual
/// information.
///
/// The existence of this function ensures that:
/// - concept dispatch tables are complete,
/// - accidental invocation is detected early and explicitly,
/// - no silent no-op behaviour is possible.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Nil concept variant
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
/// Any invocation is treated as a programming error.
///
template <std::size_t Stage, std::size_t Section, NilType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void NilOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) noexcept(false) {

    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (nilApplicable<Stage, Section, Variant>()) {

        // Debug output
        MARS2GRIB_LOG_CONCEPT(nil);

        // Successful no-op
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(nil, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

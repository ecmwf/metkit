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
/// @file referenceTimeConceptDescriptor.h
/// @brief Compile-time registry entry for the GRIB `referenceTime` concept.
///
/// This header defines `ReferenceTimeConcept`, the **compile-time descriptor**
/// that registers the GRIB `referenceTime` concept into the mars2grib
/// compile-time registry engine.
///
/// The descriptor provides:
/// - The concept name
/// - The mapping between variants and their symbolic names
/// - The set of callbacks associated with each encoding phase
/// - The entry-level matcher used to activate the concept
///
/// This file contains **no runtime logic**. All decisions are resolved
/// at compile time through template instantiation.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System include
#include <cstddef>

// Registry engine
#include "metkit/mars2grib/backend/compile-time-registry-engine/RegisterEntryDescriptor.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeEncoding.h"
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeEnum.h"
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

// Importing the compile-time registry engine namespace locally to avoid
// excessive verbosity in template-heavy code. This is restricted to an
// internal scope and not exposed through public headers.
using namespace metkit::mars2grib::backend::compile_time_registry_engine;

///
/// @brief Compile-time descriptor for the `referenceTime` concept.
///
/// `ReferenceTimeConcept` registers the GRIB `referenceTime` concept into the
/// compile-time registry engine.
///
struct ReferenceTimeConcept : RegisterEntryDescriptor<ReferenceTimeType, ReferenceTimeList> {

    ///
    /// @brief Return the canonical name of the concept.
    ///
    /// This name is used for:
    /// - registry identification
    /// - diagnostics and logging
    /// - debug and introspection utilities
    ///
    static constexpr std::string_view entryName() { return referenceTimeName; }

    ///
    /// @brief Return the symbolic name of a concept variant.
    ///
    /// @tparam T Variant enumeration value
    /// @return String view representing the variant name
    ///
    template <ReferenceTimeType T>
    static constexpr std::string_view variantName() {
        return referenceTimeTypeName<T>();
    }

    ///
    /// @brief Return the callback associated with a specific encoding phase.
    ///
    /// This hook is queried by the registry engine to obtain the function
    /// implementing the `referenceTime` concept for a given:
    /// - capability
    /// - stage
    /// - GRIB section
    /// - variant
    ///
    /// The function returns either a valid function pointer or `nullptr` if the
    /// combination is not applicable.
    ///
    /// @tparam Capability Encoding capability index
    /// @tparam Stage      Encoding stage
    /// @tparam Sec        GRIB section
    /// @tparam Variant    Concept variant
    ///
    template <std::size_t Capability, std::size_t Stage, std::size_t Sec, ReferenceTimeType Variant, class MarsDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> phaseCallbacks() {

        if constexpr (Capability == 0) {

            if constexpr (referenceTimeApplicable<Stage, Sec, Variant>()) {
                return &ReferenceTimeOp<Stage, Sec, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
            }
            else {
                return nullptr;
            }
        }
        else {
            return nullptr;
        }

        mars2gribUnreachable();
    }

    template <std::size_t Capability, ReferenceTimeType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> variantCallbacks() {
        return nullptr;
    }

    ///
    /// @brief Entry-level matcher callback.
    ///
    /// This hook is invoked to determine whether the `referenceTime` concept should be
    /// activated for a given request.
    ///
    /// @tparam Capability Matching/encoding capability index
    /// @return Matcher function pointer, or `nullptr` if not participating.
    ///
    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks() {
        if constexpr (Capability == 0) {
            return &referenceTimeMatcher<MarsDict_t, OptDict_t>;
        }
        else {
            return nullptr;
        }
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

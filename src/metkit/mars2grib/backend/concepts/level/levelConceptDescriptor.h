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
/// @file LevelConcept.h
/// @brief Compile-time registry entry for the GRIB `level` concept.
///
/// This header defines `LevelConcept`, the **compile-time descriptor**
/// that registers the GRIB `level` concept into the mars2grib
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
#include "metkit/mars2grib/backend/concepts/level/levelEncoding.h"
#include "metkit/mars2grib/backend/concepts/level/levelEnum.h"
#include "metkit/mars2grib/backend/concepts/level/levelMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

// Importing the compile-time registry engine namespace locally to avoid
// excessive verbosity in template-heavy code. This is restricted to an
// internal scope and not exposed through public headers.
using namespace metkit::mars2grib::backend::compile_time_registry_engine;

///
/// @brief Compile-time descriptor for the `level` concept.
///
/// `LevelConcept` registers the GRIB `level` concept into the
/// compile-time registry engine.
///
/// The descriptor defines:
/// - The canonical concept name
/// - The mapping from variant enum values to symbolic names
/// - The callbacks associated with each encoding phase
/// - The entry-level matcher used to detect applicability
///
/// All functions in this descriptor are `constexpr` and are evaluated
/// entirely at compile time.
///
struct LevelConcept : RegisterEntryDescriptor<LevelType, LevelList> {

    ///
    /// @brief Return the canonical name of the concept.
    ///
    /// This name is used for:
    /// - Registry identification
    /// - Diagnostics and logging
    /// - Debug and introspection facilities
    ///
    static constexpr std::string_view entryName() { return levelName; }

    ///
    /// @brief Return the symbolic name of a concept variant.
    ///
    /// @tparam T Variant enumeration value
    ///
    /// @return String view representing the variant name
    ///
    template <LevelType T>
    static constexpr std::string_view variantName() {
        return levelTypeName<T>();
    }

    ///
    /// @brief Return the callback associated with a specific encoding phase.
    ///
    /// This function is queried by the registry engine to obtain the
    /// callback implementing the `level` concept for a given:
    ///
    /// - Capability
    /// - Encoding stage
    /// - GRIB section
    /// - Concept variant
    ///
    /// The function returns:
    /// - A valid function pointer if the concept is applicable
    /// - `nullptr` otherwise
    ///
    /// @tparam Capability Encoding capability index
    /// @tparam Stage      Encoding stage
    /// @tparam Sec        GRIB section
    /// @tparam Variant    Concept variant
    /// @tparam MarsDict_t Type of MARS dictionary
    /// @tparam ParDict_t  Type of parameter dictionary
    /// @tparam OptDict_t  Type of options dictionary
    /// @tparam OutDict_t  Type of output GRIB dictionary
    ///
    /// @return Function pointer implementing the phase, or `nullptr`
    ///
    template <std::size_t Capability, std::size_t Stage, std::size_t Sec, LevelType Variant, class MarsDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> phaseCallbacks() {

        if constexpr (Capability == 0) {

            if constexpr (levelApplicable<Stage, Sec, Variant>()) {
                return &LevelOp<Stage, Sec, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
            }
            else {
                return nullptr;
            }
        }
        else {
            return nullptr;
        }

        // Avoid compiler warnings
        mars2gribUnreachable();
    }

    ///
    /// @brief Variant-specific callbacks (not used for this concept).
    ///
    /// This hook is provided for completeness of the registry interface.
    /// The `level` concept does not define variant-level callbacks,
    /// so this function always returns `nullptr`.
    ///
    /// @tparam Capability Encoding capability index
    /// @tparam Variant    Concept variant
    /// @tparam MarsDict_t Type of MARS dictionary
    /// @tparam ParDict_t  Type of parameter dictionary
    /// @tparam OptDict_t  Type of options dictionary
    /// @tparam OutDict_t  Type of output GRIB dictionary
    ///
    /// @return Always `nullptr`
    ///
    template <std::size_t Capability, LevelType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> variantCallbacks() {
        return nullptr;
    }

    ///
    /// @brief Entry-level matcher callback.
    ///
    /// This callback is invoked to determine whether the `level`
    /// concept should be activated for a given encoding request.
    ///
    /// @tparam MarsDict_t Type of MARS dictionary
    /// @tparam OptDict_t  Type of options dictionary
    ///
    /// @return Matcher function pointer
    ///
    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks() {
        if constexpr (Capability == 0) {
            return &levelMatcher<MarsDict_t, OptDict_t>;
        }
        else {
            return nullptr;
        }
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

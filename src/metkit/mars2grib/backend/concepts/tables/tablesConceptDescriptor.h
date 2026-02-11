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
/// @file TablesConcept.h
/// @brief Compile-time registry entry for the GRIB `tables` concept.
///
/// This header defines `TablesConcept`, the **compile-time descriptor**
/// that registers the GRIB `tables` concept into the mars2grib
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

// Core concept includes
#include "metkit/mars2grib/backend/concepts/tables/tablesEncoding.h"
#include "metkit/mars2grib/backend/concepts/tables/tablesEnum.h"
#include "metkit/mars2grib/backend/concepts/tables/tablesMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

// Importing the compile-time registry engine namespace locally to avoid
// excessive verbosity in template-heavy code. This is restricted to an
// internal scope and not exposed through public headers.
using namespace metkit::mars2grib::backend::compile_time_registry_engine;

///
/// @brief Compile-time descriptor for the `tables` concept.
///
/// `TablesConcept` registers the GRIB `tables` concept into the
/// compile-time registry engine.
///
struct TablesConcept : RegisterEntryDescriptor<TablesType, TablesList> {

    static constexpr std::string_view entryName() { return tablesName; }

    template <TablesType T>
    static constexpr std::string_view variantName() {
        return tablesTypeName<T>();
    }

    template <std::size_t Capability, std::size_t Stage, std::size_t Sec, TablesType Variant, class MarsDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> phaseCallbacks() {

        if constexpr (Capability == 0) {

            if constexpr (tablesApplicable<Stage, Sec, Variant>()) {
                return &TablesOp<Stage, Sec, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
            }
            else {
                return nullptr;
            }
        }
        else {
            return nullptr;
        }

        __builtin_unreachable();
    }

    template <std::size_t Capability, TablesType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> variantCallbacks() {
        return nullptr;
    }

    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks() {
        if constexpr (Capability == 0) {
            return &tablesMatcher<MarsDict_t, OptDict_t>;
        }
        else {
            return nullptr;
        }
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

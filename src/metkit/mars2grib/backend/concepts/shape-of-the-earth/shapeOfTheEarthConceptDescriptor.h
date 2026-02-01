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
 * @file ShapeOfTheEarthConcept.h
 * @brief Compile-time registry entry for the GRIB `shapeOfTheEarth` concept.
 *
 * This header defines `ShapeOfTheEarthConcept`, the **compile-time descriptor**
 * that registers the GRIB `shapeOfTheEarth` concept into the mars2grib
 * compile-time registry engine.
 *
 * The descriptor provides:
 * - The concept name
 * - The mapping between variants and their symbolic names
 * - The set of callbacks associated with each encoding phase
 * - The entry-level matcher used to activate the concept
 *
 * This file contains **no runtime logic**. All decisions are resolved
 * at compile time through template instantiation.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System include
#include <cstddef>

// Registry engine
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/RegisterEntryDescriptor.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarthEncoding.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarthMatcher.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarthEnum.h"

namespace metkit::mars2grib::backend::concepts_ {

// Importing the compile-time registry engine namespace locally to avoid
// excessive verbosity in template-heavy code. This is restricted to an
// internal scope and not exposed through public headers.
using namespace metkit::mars2grib::backend::compile_time_registry_engine;

/**
 * @brief Compile-time descriptor for the `shapeOfTheEarth` concept.
 *
 * `ShapeOfTheEarthConcept` registers the GRIB `shapeOfTheEarth` concept into the
 * compile-time registry engine.
 */
struct ShapeOfTheEarthConcept
    : RegisterEntryDescriptor<ShapeOfTheEarthType, ShapeOfTheEarthList> {

    static constexpr std::string_view entryName() {
        return shapeOfTheEarthName;
    }

    template <ShapeOfTheEarthType T>
    static constexpr std::string_view variantName() {
        return shapeOfTheEarthTypeName<T>();
    }

    template <
        std::size_t Capability,
        std::size_t Stage,
        std::size_t Sec,
        ShapeOfTheEarthType Variant,
        class MarsDict_t,
        class ParDict_t,
        class OptDict_t,
        class OutDict_t>
    static constexpr Fn<
        MarsDict_t,
        ParDict_t,
        OptDict_t,
        OutDict_t>
    phaseCallbacks() {

        if constexpr (Capability == 0) {

            if constexpr (shapeOfTheEarthApplicable<Stage, Sec, Variant>()) {
                return &ShapeOfTheEarthOp<
                    Stage,
                    Sec,
                    Variant,
                    MarsDict_t,
                    ParDict_t,
                    OptDict_t,
                    OutDict_t>;
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

    template <
        std::size_t Capability,
        ShapeOfTheEarthType Variant,
        class MarsDict_t,
        class ParDict_t,
        class OptDict_t,
        class OutDict_t>
    static constexpr Fn<
        MarsDict_t,
        ParDict_t,
        OptDict_t,
        OutDict_t>
    variantCallbacks() {
        return nullptr;
    }

    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t>
    entryCallbacks() {
        if constexpr ( Capability == 0 ) {
            return &shapeOfTheEarthMatcher<MarsDict_t, OptDict_t>;
        }
        else {
            return nullptr;
        }
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

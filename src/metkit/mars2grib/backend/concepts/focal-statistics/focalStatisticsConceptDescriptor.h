// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file focalStatisticsConceptDescriptor.h
/// @brief Compile-time registry entry for the GRIB `focalStatistics` concept.
///
/// This header defines `FocalStatisticsConcept`, the **compile-time descriptor**
/// that registers the GRIB `focalStatistics` concept into the mars2grib
/// compile-time registry engine.
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
#include "metkit/mars2grib/backend/concepts/focal-statistics/focalStatisticsEncoding.h"
#include "metkit/mars2grib/backend/concepts/focal-statistics/focalStatisticsEnum.h"
#include "metkit/mars2grib/backend/concepts/focal-statistics/focalStatisticsMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

// Importing the compile-time registry engine namespace locally to avoid
// excessive verbosity in template-heavy code. This is restricted to an
// internal scope and not exposed through public headers.
using namespace metkit::mars2grib::backend::compile_time_registry_engine;

///
/// @brief Compile-time descriptor for the `focalStatistics` concept.
///
/// The descriptor defines:
/// - The canonical concept name
/// - The mapping from variant enum values to symbolic names
/// - The callbacks associated with each encoding phase
/// - The entry-level matcher used to detect applicability
///
struct FocalStatisticsConcept : RegisterEntryDescriptor<FocalStatisticsType, FocalStatisticsList> {

    ///
    /// @brief Return the canonical name of the concept.
    ///
    static constexpr std::string_view entryName() { return focalStatisticsName; }

    ///
    /// @brief Return the symbolic name of a concept variant.
    ///
    template <FocalStatisticsType T>
    static constexpr std::string_view variantName() {
        return focalStatisticsTypeName<T>();
    }

    ///
    /// @brief Return the callback associated with a specific encoding phase.
    ///
    /// The function returns a valid function pointer if the concept is
    /// applicable, `nullptr` otherwise.
    ///
    template <std::size_t Capability, std::size_t Stage, std::size_t Sec, FocalStatisticsType Variant, class MarsDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> phaseCallbacks() {

        if constexpr (Capability == 0) {

            if constexpr (focalStatisticsApplicable<Stage, Sec, Variant>()) {
                return &FocalStatisticsOp<Stage, Sec, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
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

    ///
    /// @brief Variant-specific callbacks (not used for this concept).
    ///
    template <std::size_t Capability, FocalStatisticsType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> variantCallbacks() {
        return nullptr;
    }

    ///
    /// @brief Entry-level matcher callback.
    ///
    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks() {
        if constexpr (Capability == 0) {
            return &focalStatisticsMatcher<MarsDict_t, OptDict_t>;
        }
        else {
            return nullptr;
        }
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

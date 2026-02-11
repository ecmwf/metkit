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
/// @file EncodingPlan.h
/// @brief Construction of the header encoding execution plan.
///
/// This header defines the data structures and factory function used to
/// build an **encoding plan** for GRIB header generation.
///
/// An encoding plan is a fully resolved, runtime-ready representation of
/// *what encoding callbacks must be executed*, organized by:
///
/// - Encoding stage
/// - GRIB section
///
/// The plan is derived from a resolved header layout and from the
/// compile-time encoding callback registry.
///
/// @ingroup mars2grib_frontend_header
///
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <exception>
#include <vector>

// Project includes
#include "metkit/mars2grib/backend/concepts/EncodingCallbacksRegistry.h"
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/initializers/sectionRegistry.h"
#include "metkit/mars2grib/frontend/GribHeaderLayoutData.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::frontend::header::detail {

///
/// @brief Fixed-capacity vector used to strictly avoid dynamic allocation.
///
/// `FixedVector` provides a minimal vector-like interface backed by a
/// statically allocated `std::array`.
///
/// The primary motivation for this type is to **completely avoid dynamic
/// allocation** in hot-path code that may be executed billions of times.
/// In this context:
///
/// - The maximum number of elements is small and known at compile time
/// (typically <= 22 elements)
/// - Allocating tiny dynamic vectors would result in unacceptable
/// allocation overhead and memory fragmentation
///
/// This abstraction also allows future replacement with a small-buffer
/// optimized container (e.g. `boost::container::small_vector`) without
/// changing the surrounding code.
///
/// @tparam T        Element type
/// @tparam Capacity Maximum number of elements
///
template <typename T, std::size_t Capacity>
struct FixedVector {

    /// Underlying fixed storage
    std::array<T, Capacity> data;

    /// Number of valid elements in @ref data
    std::size_t current_size = 0;

    ///
    /// @brief Append an element to the vector.
    ///
    /// @throws Mars2GribGenericException
    /// If the fixed capacity is exceeded
    ///
    void push_back(const T& value) {
        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;
        if (current_size >= Capacity) {
            throw Mars2GribGenericException("FixedVector capacity exceeded", Here());
        }
        data[current_size++] = value;
    }

    /// Remove all elements
    void clear() { current_size = 0; }

    /// Return the number of stored elements
    std::size_t size() const { return current_size; }

    /// Begin iterator (non-const)
    T* begin() { return data.data(); }

    /// End iterator (non-const)
    T* end() { return data.data() + current_size; }

    /// Begin iterator (const)
    const T* begin() const { return data.data(); }

    /// End iterator (const)
    const T* end() const { return data.data() + current_size; }

    /// Element access (non-const)
    T& operator[](std::size_t i) { return data[i]; }

    /// Element access (const)
    const T& operator[](std::size_t i) const { return data[i]; }
};


///
/// @brief Global compile-time registry of indexing and layout metadata.
///
/// `GeneralRegistry` provides **compile-time constants and indexing
/// infrastructure** shared across the entire mars2grib codebase.
///
/// It defines:
/// - The total number of concepts, variants, sections, and stages
/// - Stable indices used to access registry-backed tables
///
/// Callback registries are intentionally separated from `GeneralRegistry`:
/// - They are heavily templated on dictionary types
/// - In many contexts only index metadata is required
/// - Templating on unused dictionary parameters would be unnecessary
/// and harmful to compile-time and readability
///
/// This separation keeps indexing lightweight while allowing callback
/// registries to remain fully type-safe where required.
///
using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;

///
/// @brief Encoding callbacks registry alias.
///
/// This alias refers to the compile-time registry that maps
/// `(variant, section, stage)` to concrete encoding callbacks.
///
/// The underscore namespace is used to avoid clashes with the C++20
/// `concepts` keyword.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using EncodingRegistry =
    metkit::mars2grib::backend::concepts_::EncodingCallbacksRegistry<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;

///
/// @brief Alias for the encoding callback function type.
///
/// Extracted from the encoding callbacks registry.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using Fn_t = typename EncodingRegistry<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>::Fn_t;

///
/// @brief Encoding execution plan.
///
/// An `EncodingPlan` is a two-dimensional grid indexed by:
///
/// - Encoding stage
/// - GRIB section
///
/// Each cell contains a fixed-capacity list of encoding callbacks
/// that must be executed for that `(stage, section)` pair.
///
/// Layout:
///
/// @code
/// EncodingPlan[Stage][Section] -> list of callbacks
/// @endcode
///
/// Stage `0` is reserved for **section initializers**.
/// Stages `1..N` contain concept encoding callbacks.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using EncodingPlan =
    std::array<std::array<FixedVector<Fn_t<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, GeneralRegistry::NConcepts>,
                          GeneralRegistry::NSections>,
               GeneralRegistry::NStages + 1>;


///
/// @brief Build an encoding plan from resolved header layout data.
///
/// This factory function constructs a complete `EncodingPlan` by:
///
/// 1. Selecting section initializer callbacks (stage 0)
/// 2. Selecting concept encoding callbacks for each section and stage
///
/// The plan is fully determined by:
/// - The resolved header layout (template numbers and variants)
/// - The compile-time encoding callbacks registry
///
/// @tparam MarsDict_t Type of MARS dictionary
/// @tparam ParDict_t  Type of parameter dictionary
/// @tparam OptDict_t  Type of options dictionary
/// @tparam OutDict_t  Type of output GRIB dictionary
///
/// @param[in] headerLayout Resolved header layout data
///
/// @return Fully populated encoding plan
///
/// @throws Mars2GribFrontendException
/// If plan construction fails
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
EncodingPlan<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> make_EncodingPlan_or_throw(
    const GribHeaderLayoutData& headerLayout) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Access the static callback registry
        const auto& callbacks = EncodingRegistry<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>::encodingCallbacks;
        EncodingPlan<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> table;

        // Stage 0: Populate section initializers (always one initializer per section)
        for (std::size_t sid = 0; sid < GeneralRegistry::NSections; ++sid) {
            std::size_t templ = headerLayout.sectionLayouts[sid].templateNumber;
            table[0][sid].push_back(
                metkit::mars2grib::backend::sections::initializers::sectionRegistry<MarsDict_t, ParDict_t, OptDict_t,
                                                                                    OutDict_t>(sid, templ));
        }

        // Stages 1 to N: Populate encoding callbacks
        for (std::size_t pid = 0; pid < GeneralRegistry::NStages; ++pid) {
            for (std::size_t sid = 0; sid < GeneralRegistry::NSections; ++sid) {
                const auto& section = headerLayout.sectionLayouts[sid];

                // Index pid+1 because stage 0 is reserved for initializers
                auto& cell = table[pid + 1][sid];
                cell.clear();

                for (std::size_t cid = 0; cid < section.count; ++cid) {
                    std::size_t vid = section.variantIndices[cid];
                    const auto& f   = callbacks[vid][sid][pid];

                    if (f) {
                        cell.push_back(f);
                    }
                }
            }
        }
        return table;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Unable to create encoding plan", Here()));
    }
}

}  // namespace metkit::mars2grib::frontend::header::detail
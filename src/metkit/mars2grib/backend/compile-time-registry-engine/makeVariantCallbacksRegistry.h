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
/// @file VariantCallbacksRegistry.h
/// @brief Compile-time global registry for variant-level callbacks.
///
/// This header defines the machinery required to **materialize, entirely at
/// compile time, a dense dispatch table of variant-level callbacks**.
///
/// -----------------------------------------------------------------------------
/// Conceptual result
/// -----------------------------------------------------------------------------
///
/// The primary product of this header is a constexpr array with the following
/// logical structure:
///
/// @code
/// variantCallbacks[globalVariantIndex] -> Fn | nullptr
/// @endcode
///
/// where:
///
/// - `globalVariantIndex` is a **flattened index** spanning *all variants of all
/// entries*, in a single contiguous index space.
/// - `Fn` is a function pointer type implementing a concrete encoding operation.
/// - `nullptr` denotes that the requested capability is **not supported** for
/// that specific variant.
///
/// -----------------------------------------------------------------------------
/// Definition of the global variant index space
/// -----------------------------------------------------------------------------
///
/// The global variant index space is defined *structurally* and *deterministically*
/// by two independent compile-time orderings:
///
/// 1. **Entry ordering**
/// The order of Entry descriptors in `EntriesList` (a `TypeList`).
///
/// 2. **Variant ordering**
/// For each Entry, the order of values in `Entry::VariantList`
/// (a `ValueList`).
///
/// The resulting index space is:
///
/// @code
/// [ Entry0::Variant0,
/// Entry0::Variant1,
/// ...
/// Entry1::Variant0,
/// Entry1::Variant1,
/// ...
/// ]
/// @endcode
///
/// This ordering is guaranteed to be:
/// - contiguous
/// - stable across translation units
/// - independent of compilation order
///
/// -----------------------------------------------------------------------------
/// Capability selection model
/// -----------------------------------------------------------------------------
///
/// Each variant may expose zero or more *capabilities*.
///
/// A capability is identified by a **compile-time `std::size_t` constant**.
/// Capability selection is resolved entirely at compile time, producing a
/// specialized dispatch table for each capability.
///
/// -----------------------------------------------------------------------------
/// Design goals
/// -----------------------------------------------------------------------------
///
/// - **Zero runtime branching**
/// No conditionals are required when invoking variant callbacks.
///
/// - **No dynamic allocation**
/// All data structures are `constexpr std::array`.
///
/// - **Header-only**
/// Safe to include in any number of translation units.
///
/// - **Dense layout**
/// One slot per variant, indexed directly by global variant ID.
///
/// -----------------------------------------------------------------------------
/// Assumptions and invariants
/// -----------------------------------------------------------------------------
///
/// Each Entry type appearing in `EntriesList` is assumed to provide:
///
/// @code
/// template <
/// std::size_t Capability,
/// auto Variant,
/// class MarsDict_t,
/// class ParDict_t,
/// class OptDict_t,
/// class OutDict_t
/// >
/// static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>
/// variantCallbacks();
/// @endcode
///
/// returning either:
/// - a valid function pointer implementing the capability for that variant, or
/// - `nullptr` if the capability is not supported.
///
/// Failure to meet this contract results in a compile-time error.
///
/// -----------------------------------------------------------------------------
/// Relationship to other registries
/// -----------------------------------------------------------------------------
///
/// This registry operates at **variant granularity**.
///
/// It is orthogonal to:
/// - entry-level callback registries
/// - concept/variant indexing registries
///
/// Those registries define *index spaces*; this registry defines *behavior*.
///
/// @ingroup mars2grib_backend_compile_time_registry_engine
///

#pragma once

// System includes
#include <array>
#include <cstddef>
#include <utility>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/RegisterEntryDescriptor.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"


namespace metkit::mars2grib::backend::compile_time_registry_engine {

///
/// @namespace detail
/// @brief Internal implementation details of the variant callbacks registry.
///
/// This namespace contains the low-level template machinery required to:
/// - instantiate per-variant callbacks
/// - concatenate per-entry variant blocks
/// - produce the final flattened dispatch table
///
/// All entities in this namespace are **implementation details** and must not be
/// used directly outside this header.
///
namespace detail {

///
/// @brief Alias for a variant-level callback function pointer.
///
/// This alias represents the *callable unit* stored in the variant callbacks
/// registry.
///
/// The function signature is determined by the concrete dictionary types.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the output dictionary
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using VariantCallback = Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;


///
/// @brief Build the variant-level callback block for a single Entry.
///
/// This function materializes a contiguous array of callbacks corresponding
/// to **all variants of a single Entry**, in the order defined by
/// `Entry::VariantList`.
///
/// Each element of the returned array corresponds to exactly one variant.
///
/// @tparam Entry       Entry descriptor type
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam ParDict_t   Parameter dictionary type
/// @tparam OptDict_t   Options dictionary type
/// @tparam OutDict_t   Output dictionary type
/// @tparam Variants    Pack of variant enum values
///
/// @param[in]  ValueList<Variants...>
/// Compile-time list defining variant order
///
/// @return
/// A `constexpr std::array` of size `sizeof...(Variants)` containing
/// the callbacks for each variant.
///
/// @note
/// Each callback may be `nullptr` if the capability is not supported
/// for the corresponding variant.
///
template <class Entry, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t,
          auto... Variants>
constexpr std::array<VariantCallback<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, sizeof...(Variants)>
makeEntryVariantCallbacks(ValueList<Variants...>) {
    return {{Entry::template variantCallbacks<Capability, Variants, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>()...}};
}

///
/// @brief Convenience wrapper to build the variant callback block for an Entry.
///
/// This overload extracts `Entry::VariantList` automatically and forwards
/// to the ValueList-based implementation.
///
template <class Entry, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
constexpr auto makeEntryVariantCallbacks() {
    return makeEntryVariantCallbacks<Entry, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(
        typename Entry::VariantList{});
}


///
/// @brief Compile-time builder for the full variant callbacks table.
///
/// This metafunction recursively traverses the `EntriesList` typelist and
/// concatenates the per-entry variant callback blocks into a single,
/// flattened dispatch table.
///
/// The resulting table:
/// - has one slot per global variant
/// - preserves Entry ordering
/// - preserves VariantList ordering within each Entry
///
/// @tparam EntriesList TypeList of Entry descriptors
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam ParDict_t   Parameter dictionary type
/// @tparam OptDict_t   Options dictionary type
/// @tparam OutDict_t   Output dictionary type
///
template <class EntriesList, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct BuildVariantCallbacks;

///
/// @brief Base case for an empty EntriesList.
///
/// Produces an empty constexpr array.
///
/// This specialization terminates the recursion.
///
template <std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
struct BuildVariantCallbacks<TypeList<>, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t> {
    static constexpr std::array<VariantCallback<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, 0> value() { return {}; }
};

///
/// @brief Recursive case: prepend Head’s variant block and recurse on Tail.
///
/// This specialization:
/// - builds the variant callback block for `Head`
/// - recursively builds the table for `Tail...`
/// - concatenates the two blocks using `concat`
///
/// The order of the resulting array is strictly deterministic.
///
template <class Head, class... Tail, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct BuildVariantCallbacks<TypeList<Head, Tail...>, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t> {
    static constexpr auto value() {
        constexpr auto head =
            makeEntryVariantCallbacks<Head, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>();

        constexpr auto tail =
            BuildVariantCallbacks<TypeList<Tail...>, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>::value();

        return concat(head, tail);
    }
};

}  // namespace detail

///
/// @brief Construct the compile-time variant callbacks registry.
///
/// This function is the **public API** of the variant callbacks registry.
///
/// It materializes, at compile time, a dense array of variant-level callbacks
/// corresponding to:
/// - a fixed EntriesList
/// - a fixed capability
/// - concrete dictionary types
///
/// @tparam EntriesList TypeList of Entry descriptors
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam ParDict_t   Parameter dictionary type
/// @tparam OptDict_t   Options dictionary type
/// @tparam OutDict_t   Output dictionary type
///
/// @return
/// A `constexpr std::array<Fn<...>, N>` where:
/// - `N` equals the total number of variants across all Entries
/// - index `i` corresponds to global variant index `i`
///
/// @note
/// The returned array is intended to be:
/// - stored as a `static constexpr` object
/// - indexed directly in hot paths
/// - never modified
///
template <class EntriesList, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
constexpr auto makeVariantCallbacksRegistry() {
    return detail::BuildVariantCallbacks<EntriesList, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>::value();
}

}  // namespace metkit::mars2grib::backend::compile_time_registry_engine

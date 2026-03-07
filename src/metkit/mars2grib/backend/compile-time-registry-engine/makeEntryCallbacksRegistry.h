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
/// @file EntryCallbacksRegistry.h
/// @brief Compile-time global registry for entry-level callbacks.
///
/// This header defines the machinery required to **materialize, at compile time,
/// a dense lookup table of entry-level callbacks**, indexed by a *capability ID*
/// and by *entry index*.
///
/// -----------------------------------------------------------------------------
/// Conceptual model
/// -----------------------------------------------------------------------------
///
/// Given:
///
/// - a compile-time list of Entry descriptors (`EntriesList`)
/// - a compile-time capability identifier (`Capability`)
/// - concrete dictionary types (`MarsDict_t`, `OptDict_t`)
///
/// this facility builds a constexpr table of the form:
///
/// @code
/// entryCallbacks[entryIndex] -> Fm<MarsDict_t, OptDict_t> | nullptr
/// @endcode
///
/// where:
///
/// - `entryIndex` is the index of the Entry in `EntriesList`
/// - `Fm<MarsDict_t, OptDict_t>` is a function pointer type
/// - `nullptr` denotes “capability not supported by this entry”
///
/// -----------------------------------------------------------------------------
/// Design goals
/// -----------------------------------------------------------------------------
///
/// - **Pure compile-time construction**
/// All tables are built via constexpr recursion and concatenation.
///
/// - **Zero runtime branching**
/// Capability selection is resolved entirely at compile time.
///
/// - **Dense layout**
/// One callback slot per entry, preserving EntryList order.
///
/// - **Header-only**
/// Safe for inclusion in multiple translation units.
///
/// -----------------------------------------------------------------------------
/// Assumptions and invariants
/// -----------------------------------------------------------------------------
///
/// Each Entry type in `EntriesList` is assumed to provide:
///
/// @code
/// template <std::size_t Capability, class MarsDict_t, class OptDict_t>
/// static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks();
/// @endcode
///
/// returning either:
/// - a valid function pointer implementing the capability, or
/// - `nullptr` if the capability is not supported.
///
/// Failure to meet this contract results in a compile-time error.
///
#pragma once

///
/// @file EntryCallbacksRegistry.h
/// @brief Compile-time global registry for entry-level callbacks.
///
/// Builds a compile-time table:
///
/// entryCallbacks[entryIndex] -> Fm | nullptr
///
/// Capability is selected via a compile-time std::size_t index.
///

// System includes
#include <array>
#include <cstddef>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/RegisterEntryDescriptor.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/utils.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::compile_time_registry_engine {

///
/// @namespace detail
/// @brief Internal implementation details of the entry callbacks registry.
///
/// This namespace contains low-level templates used to:
/// - instantiate per-entry callbacks
/// - recursively concatenate them into a flat constexpr array
///
/// All entities in this namespace are considered **implementation details**
/// and must not be used directly outside this header.
///
namespace detail {

///
/// @brief Instantiate the callback for a single Entry and capability.
///
/// This function delegates to the Entry’s static `entryCallbacks` interface
/// and returns the function pointer corresponding to the requested capability.
///
/// @tparam Entry       Entry descriptor type
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam OptDict_t   Options dictionary type
///
/// @return
/// A function pointer of type `Fm<MarsDict_t, OptDict_t>`, or `nullptr`
/// if the Entry does not implement the given capability.
///
/// @note
/// This function is `constexpr` and intended to be invoked only during
/// compile-time table construction.
///
template <class Entry, std::size_t Capability, class MarsDict_t, class OptDict_t>
constexpr Fm<MarsDict_t, OptDict_t> makeEntryCallback() {
    return Entry::template entryCallbacks<Capability, MarsDict_t, OptDict_t>();
}

///
/// @brief Compile-time builder for the entry callbacks table.
///
/// This metafunction recursively traverses the `EntriesList` typelist and
/// concatenates the callback corresponding to each Entry into a flat array.
///
/// The resulting array has:
/// - size equal to `EntriesList::size`
/// - element `i` corresponding to the i-th Entry in the typelist
///
/// @tparam EntriesList TypeList of Entry descriptors
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam OptDict_t   Options dictionary type
///
template <class EntriesList, std::size_t Capability, class MarsDict_t, class OptDict_t>
struct BuildEntryCallbacks;


///
/// @brief Base case for an empty EntriesList.
///
/// Produces an empty constexpr array.
///
template <std::size_t Capability, class MarsDict_t, class OptDict_t>
struct BuildEntryCallbacks<TypeList<>, Capability, MarsDict_t, OptDict_t> {
    static constexpr std::array<Fm<MarsDict_t, OptDict_t>, 0> value() { return {}; }
};

///
/// @brief Recursive case: prepend callback for Head and recurse on Tail.
///
/// This specialization:
/// - builds a one-element array containing Head’s callback
/// - recursively builds the tail array
/// - concatenates the two using `concat`
///
/// The order of Entries in the TypeList is strictly preserved.
///
template <class Head, class... Tail, std::size_t Capability, class MarsDict_t, class OptDict_t>
struct BuildEntryCallbacks<TypeList<Head, Tail...>, Capability, MarsDict_t, OptDict_t> {
    static constexpr auto value() {
        constexpr auto head =
            std::array<Fm<MarsDict_t, OptDict_t>, 1>{makeEntryCallback<Head, Capability, MarsDict_t, OptDict_t>()};

        constexpr auto tail = BuildEntryCallbacks<TypeList<Tail...>, Capability, MarsDict_t, OptDict_t>::value();

        return concat(head, tail);
    }
};

}  // namespace detail

///
/// @brief Construct the compile-time entry callbacks registry.
///
/// This is the **public API** of the entry callbacks registry.
///
/// It materializes, at compile time, a dense array of entry-level callbacks
/// corresponding to the specified capability and dictionary types.
///
/// @tparam EntriesList TypeList of Entry descriptors
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam OptDict_t   Options dictionary type
///
/// @return
/// A `constexpr std::array<Fm<MarsDict_t, OptDict_t>, N>` where:
/// - `N == EntriesList::size`
/// - element `i` is the callback for the i-th Entry
///
/// @note
/// The returned array is intended to be:
/// - stored as a `static constexpr` object
/// - indexed directly in hot paths without branching
///
template <class EntriesList, std::size_t Capability, class MarsDict_t, class OptDict_t>
constexpr auto makeEntryCallbacksRegistry() {
    return detail::BuildEntryCallbacks<EntriesList, Capability, MarsDict_t, OptDict_t>::value();
}

}  // namespace metkit::mars2grib::backend::compile_time_registry_engine

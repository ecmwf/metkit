
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
/// @file PhaseCallbacksRegistry.h
/// @brief Compile-time global registry for phase-level callbacks.
///
/// This header defines the machinery required to **materialize, entirely at
/// compile time, a three-dimensional dispatch table of phase-level callbacks**.
///
/// -----------------------------------------------------------------------------
/// Conceptual result
/// -----------------------------------------------------------------------------
///
/// The primary product of this header is a constexpr data structure with the
/// following logical shape:
///
/// @code
/// phaseCallbacks[globalVariant][stage][section] -> Fn | nullptr
/// @endcode
///
/// where:
///
/// - `globalVariant` is a flattened index over *all variants of all entries*
/// (as defined by the compile-time registry engine).
/// - `stage` is an encoding stage index in the range `[0, NUM_STAGES)`.
/// - `section` is a GRIB section index in the range `[0, NUM_SECTIONS)`.
/// - `Fn` is a function pointer implementing a concrete encoding action.
/// - `nullptr` denotes that the requested capability is **not implemented**
/// for the given `(variant, stage, section)` triple.
///
/// -----------------------------------------------------------------------------
/// Structural definition of the index space
/// -----------------------------------------------------------------------------
///
/// The three axes of the dispatch table are defined as follows:
///
/// 1. **Global variant axis**
/// Determined by:
/// - the order of Entry descriptors in `EntriesList` (TypeList order),
/// - the order of variant values in each `Entry::VariantList` (ValueList order).
///
/// 2. **Stage axis**
/// A fixed compile-time dimension of size `NUM_STAGES`, representing
/// the sequential encoding phases of the GRIB header construction pipeline.
///
/// 3. **Section axis**
/// A fixed compile-time dimension of size `NUM_SECTIONS`, representing
/// GRIB message sections.
///
/// The resulting layout is:
///
/// @code
/// [
/// Variant0 -> [ Stage0 -> [Sec0, Sec1, ...],
/// Stage1 -> [Sec0, Sec1, ...],
/// ... ],
/// Variant1 -> [ ... ],
/// ...
/// ]
/// @endcode
///
/// All dimensions are:
/// - contiguous,
/// - fixed at compile time,
/// - deterministic,
/// - identical across all translation units.
///
/// -----------------------------------------------------------------------------
/// Capability selection model
/// -----------------------------------------------------------------------------
///
/// Phase-level behavior is parameterized by a **compile-time capability
/// identifier**, represented as a `std::size_t` template parameter.
///
/// Each capability produces a *distinct* registry instantiation.
/// Capability selection therefore incurs:
/// - zero runtime cost,
/// - zero branching,
/// - zero dynamic dispatch.
///
/// -----------------------------------------------------------------------------
/// Design goals
/// -----------------------------------------------------------------------------
///
/// - **Zero runtime overhead**
/// All decisions are resolved at compile time.
///
/// - **Dense, indexable layout**
/// Direct indexing without indirection or lookup structures.
///
/// - **Strict structural alignment**
/// Indices are guaranteed to match those produced by the variant and
/// entry registries.
///
/// - **Header-only, constexpr-only**
/// Safe for inclusion in any translation unit.
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
/// auto Stage,
/// auto Section,
/// auto Variant,
/// class MarsDict_t,
/// class ParDict_t,
/// class OptDict_t,
/// class OutDict_t
/// >
/// static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>
/// phaseCallbacks();
/// @endcode
///
/// returning either:
/// - a valid function pointer implementing the phase-level operation, or
/// - `nullptr` if the operation is not defined.
///
/// Any deviation from this contract results in a compile-time error.
///
/// -----------------------------------------------------------------------------
/// Relationship to other registries
/// -----------------------------------------------------------------------------
///
/// This registry represents the **final and most granular dispatch layer**.
///
/// It composes and refines:
/// - EntryVariantRegistry   (index space definition),
/// - EntryCallbacksRegistry (entry-level behavior),
/// - VariantCallbacksRegistry (variant-level behavior).
///
/// This layer adds:
/// - stage awareness,
/// - section awareness,
/// - full phase resolution.
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
/// @brief Internal implementation details of the phase callbacks registry.
///
/// This namespace contains low-level template machinery used to:
/// - build per-section rows,
/// - assemble per-stage planes,
/// - concatenate all variant blocks across all entries.
///
/// All entities in this namespace are **implementation details** and must not be
/// referenced directly by user code.
///
namespace detail {

///
/// @brief One row of phase callbacks for a fixed stage.
///
/// A PhaseRow represents all section-level callbacks for a single
/// `(variant, stage)` pair.
///
/// Layout:
/// @code
/// PhaseRow[section] -> Fn | nullptr
/// @endcode
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using PhaseRow = std::array<Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, NUM_SECTIONS>;

///
/// @brief One plane of phase callbacks for a fixed variant.
///
/// A PhasePlane aggregates all stages and sections for a single variant.
///
/// Layout:
/// @code
/// PhasePlane[stage][section] -> Fn | nullptr
/// @endcode
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using PhasePlane = std::array<PhaseRow<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, NUM_STAGES>;


///
/// @brief Build a single phase row for a fixed Entry, Variant, and Stage.
///
/// This function instantiates all section-level callbacks corresponding to:
/// - one Entry,
/// - one Variant,
/// - one Stage,
/// across all sections.
///
/// The resulting PhaseRow has exactly `NUM_SECTIONS` elements.
///
template <class Entry, std::size_t Capability, auto Stage, auto Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t, std::size_t... Secs>
constexpr PhaseRow<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> makePhaseRow(std::index_sequence<Secs...>) {
    return {{Entry::template phaseCallbacks<Capability, Stage, Secs, Variant, MarsDict_t, ParDict_t, OptDict_t,
                                            OutDict_t>()...}};
}

///
/// @brief Build a full phase plane for a single variant.
///
/// This function builds:
/// - one PhaseRow per stage,
/// - for all stages `[0, NUM_STAGES)`,
/// producing a complete PhasePlane.
///
template <class Entry, std::size_t Capability, auto Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t, std::size_t... Stages>
constexpr PhasePlane<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> makePhasePlane(std::index_sequence<Stages...>) {
    return {{// For every stage in Stages..., build a full row of sections
             makePhaseRow<Entry, Capability, Stages, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(
                 std::make_index_sequence<NUM_SECTIONS>{})...}};
}

///
/// @brief Build phase planes for all variants of a single Entry.
///
/// This function expands over `Entry::VariantList` and produces
/// one PhasePlane per variant.
///
/// The order of planes exactly matches the VariantList order.
///
template <class Entry, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t,
          auto... Variants>
constexpr std::array<PhasePlane<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, sizeof...(Variants)>
makeEntryPhaseCallbacks(ValueList<Variants...>) {
    return {{makePhasePlane<Entry, Capability, Variants, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(
        std::make_index_sequence<NUM_STAGES>{})...}};
}

template <class Entry, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
constexpr auto makeEntryPhaseCallbacks() {
    return makeEntryPhaseCallbacks<Entry, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(
        typename Entry::VariantList{});
}

///
/// @brief Compile-time builder for the full phase callbacks registry.
///
/// This metafunction recursively traverses the EntriesList typelist and
/// concatenates the per-entry variant phase blocks into a single,
/// flattened registry indexed by global variant ID.
///
/// The resulting array has:
/// - one PhasePlane per global variant,
/// - total size equal to the total number of variants across all entries.
///
template <class EntriesList, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct BuildPhaseCallbacks;

///
/// @brief Base case for an empty EntriesList.
///
/// Terminates recursion and produces an empty registry.
///
template <std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
struct BuildPhaseCallbacks<TypeList<>, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t> {
    static constexpr std::array<PhasePlane<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, 0> value() { return {}; }
};

///
/// @brief Recursive case: prepend Head’s phase blocks and recurse on Tail.
///
/// This preserves:
/// - Entry ordering,
/// - Variant ordering,
/// - Stage ordering,
/// - Section ordering.
///
template <class Head, class... Tail, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct BuildPhaseCallbacks<TypeList<Head, Tail...>, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t> {
    static constexpr auto value() {
        constexpr auto head = makeEntryPhaseCallbacks<Head, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>();

        constexpr auto tail =
            BuildPhaseCallbacks<TypeList<Tail...>, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>::value();

        return concat(head, tail);
    }
};

}  // namespace detail

///
/// @brief Construct the compile-time phase callbacks registry.
///
/// This function is the **public API** of the phase-level registry.
///
/// It materializes, at compile time, a fully expanded three-dimensional
/// dispatch table indexed by:
/// - global variant,
/// - stage,
/// - section.
///
/// @tparam EntriesList TypeList of Entry descriptors
/// @tparam Capability  Compile-time capability identifier
/// @tparam MarsDict_t  MARS dictionary type
/// @tparam ParDict_t   Parameter dictionary type
/// @tparam OptDict_t   Options dictionary type
/// @tparam OutDict_t   Output dictionary type
///
/// @return
/// A `constexpr std::array<PhasePlane<...>, N>` where:
/// - `N` is the total number of global variants,
/// - element `i` corresponds to global variant index `i`.
///
/// @note
/// The returned object is intended to be:
/// - stored as a `static constexpr` variable,
/// - indexed directly in hot paths,
/// - never modified.
///
template <class EntriesList, std::size_t Capability, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
constexpr auto makePhaseCallbacksRegistry() {
    return detail::BuildPhaseCallbacks<EntriesList, Capability, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>::value();
}

}  // namespace metkit::mars2grib::backend::compile_time_registry_engine

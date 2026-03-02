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
/// @file callbacksRegistry_common.h
/// @brief Common compile-time vocabulary for concept dispatch registries.
///
/// This header defines the **shared compile-time interface and vocabulary**
/// used by all dispatch registries in the mars2grib backend.
///
/// -----------------------------------------------------------------------------
/// Scope and responsibility
/// -----------------------------------------------------------------------------
///
/// This file is intentionally **minimal and declarative**.
///
/// It provides:
/// - a canonical *Entry descriptor interface*,
/// - shared naming and typing conventions,
/// - compile-time constants derived from structural metadata.
///
/// It explicitly does **not**:
/// - build dispatch tables,
/// - perform compile-time recursion,
/// - define registry logic,
/// - contain any metaprogramming algorithms.
///
/// Those responsibilities are delegated to higher-level registry headers
/// (EntryVariantRegistry, EntryCallbacksRegistry, VariantCallbacksRegistry,
/// PhaseCallbacksRegistry).
///
/// -----------------------------------------------------------------------------
/// Architectural role
/// -----------------------------------------------------------------------------
///
/// Conceptually, this header defines the **contract** that every concept
/// participating in the encoding pipeline must satisfy.
///
/// All registry engines assume that Entry descriptors conform exactly to
/// the interface specified here.
///
/// Any deviation from this contract results in:
/// - compilation failure, or
/// - silent misalignment of registry tables (undefined behavior).
///
/// -----------------------------------------------------------------------------
/// Design constraints
/// -----------------------------------------------------------------------------
///
/// - Header-only
/// - No state
/// - No runtime logic
/// - No ownership
/// - No dynamic allocation
///
/// Everything defined here must be:
/// - usable in constant expressions,
/// - safe to include in any translation unit,
/// - independent of include order (modulo dependent headers).
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

// System includes
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"

namespace metkit::mars2grib::backend::compile_time_registry_engine {

///
/// @brief Descriptor interface for a single semantic concept entry.
///
/// `RegisterEntryDescriptor` defines the **compile-time interface contract**
/// that every concept entry must implement in order to participate in:
///
/// - concept/variant indexing,
/// - entry-level dispatch,
/// - variant-level dispatch,
/// - phase-level (stage × section) dispatch.
///
/// This struct is **never instantiated**.
/// It is used purely as a *compile-time interface specification*.
///
/// -----------------------------------------------------------------------------
/// Template parameters
/// -----------------------------------------------------------------------------
///
/// @tparam VariantEnum
/// Enum type enumerating all variants of the concept.
///
/// @tparam VariantListT
/// A `ValueList<...>` containing all values of `VariantEnum`
/// that represent valid variants.
///
/// The order of values in `VariantListT` is **semantically significant** and
/// defines:
/// - local variant indices,
/// - ordering in flattened variant tables.
///
/// -----------------------------------------------------------------------------
/// Semantic meaning
/// -----------------------------------------------------------------------------
///
/// Each specialization of `RegisterEntryDescriptor` represents exactly:
///
/// - one *concept* (e.g. levelType, timeRange, gridType),
/// - a finite, ordered set of *variants*,
/// - a family of dispatch points parameterized by:
/// - capability,
/// - variant,
/// - stage,
/// - section,
/// - dictionary types.
///
/// -----------------------------------------------------------------------------
/// Required invariants
/// -----------------------------------------------------------------------------
///
/// Implementations must guarantee:
///
/// - `VariantEnum` is an enum type.
/// - `VariantListT` lists *only* values of `VariantEnum`.
/// - Every value in `VariantListT` is unique.
/// - `VariantListT::size` accurately reflects the number of variants.
///
/// Violating these invariants results in undefined behavior in registry engines.
///
template <typename VariantEnum, typename VariantListT>
struct RegisterEntryDescriptor {

    ///
    /// @brief Enum type representing the variants of this concept.
    ///
    using Variant = VariantEnum;

    ///
    /// @brief Compile-time list of all valid variant values.
    ///
    /// This list defines the *local ordering* of variants within the concept.
    ///
    using VariantList = VariantListT;

    ///
    /// @brief Return the canonical name of the concept.
    ///
    /// This name is used for:
    /// - diagnostics,
    /// - debugging output,
    /// - string-based lookup paths.
    ///
    /// @return
    /// A stable, null-terminated string view identifying the concept.
    ///
    /// @note
    /// The returned string must have static storage duration.
    ///
    static constexpr std::string_view entryName();

    ///
    /// @brief Return the canonical name of a variant.
    ///
    /// This function maps a compile-time variant value to its
    /// human-readable identifier.
    ///
    /// @tparam V
    /// A value of `VariantEnum` identifying the variant.
    ///
    /// @return
    /// A stable, null-terminated string view identifying the variant.
    ///
    /// @note
    /// This function is used exclusively in constexpr contexts and
    /// in diagnostic utilities.
    ///
    template <VariantEnum V>
    static constexpr std::string_view variantName();

    ///
    /// @brief Number of variants supported by this concept.
    ///
    /// This value is derived directly from `VariantList`.
    ///
    /// @note
    /// This constant is consumed by registry engines to:
    /// - allocate compile-time tables,
    /// - compute offsets in flattened index spaces.
    ///
    static constexpr std::size_t variant_count = VariantList::size;

    ///
    /// @brief Phase-level dispatch interface.
    ///
    /// This function provides the most granular level of dispatch:
    /// it selects a callback based on:
    /// - capability,
    /// - stage,
    /// - section,
    /// - variant,
    /// - concrete dictionary types.
    ///
    /// @tparam Capability
    /// Compile-time capability identifier.
    ///
    /// @tparam Stage
    /// Encoding stage index.
    ///
    /// @tparam Section
    /// GRIB section index.
    ///
    /// @tparam Variant
    /// Variant value of this concept.
    ///
    /// @tparam MarsDict_t
    /// MARS dictionary type.
    ///
    /// @tparam ParDict_t
    /// Parameter dictionary type.
    ///
    /// @tparam OptDict_t
    /// Options dictionary type.
    ///
    /// @tparam OutDict_t
    /// Output dictionary type.
    ///
    /// @return
    /// A function pointer implementing the requested phase-level behavior,
    /// or `nullptr` if the combination is not supported.
    ///
    template <std::size_t Capability, std::size_t Stage, std::size_t Section, VariantEnum Variant, class MarsDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> phaseCallbacks();

    ///
    /// @brief Variant-level dispatch interface.
    ///
    /// This function selects a callback based on:
    /// - capability,
    /// - variant,
    /// - concrete dictionary types.
    ///
    /// It is less granular than `phaseCallbacks` and is typically used
    /// in earlier or coarser dispatch layers.
    ///
    /// @return
    /// A function pointer implementing the variant-level behavior,
    /// or `nullptr` if unsupported.
    ///
    template <std::size_t Capability, VariantEnum Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> variantCallbacks();

    ///
    /// @brief Entry-level dispatch interface.
    ///
    /// This function selects a callback based only on:
    /// - capability,
    /// - dictionary types.
    ///
    /// It represents the coarsest dispatch granularity and is typically
    /// used for:
    /// - dictionary matching,
    /// - capability probing,
    /// - preflight checks.
    ///
    /// @return
    /// A function pointer implementing the entry-level behavior,
    /// or `nullptr` if unsupported.
    ///
    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks();
};

}  // namespace metkit::mars2grib::backend::compile_time_registry_engine

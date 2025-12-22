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
 * @file conceptCore.h
 * @brief Compile-time infrastructure for mars2grib concept registration and dispatch.
 *
 * This header defines the **core compile-time machinery** used by the mars2grib
 * backend to:
 *
 * - represent encoding stages and GRIB sections,
 * - generate fully specialized concept dispatch tables at compile time,
 * - register all concept variants into a runtime registry without dynamic
 *   branching or runtime lookup logic.
 *
 * The design is intentionally:
 * - **header-only**
 * - **pure C++17**
 * - **constexpr-driven**
 * - **allocation-free at runtime**
 *
 * No C++20 language features (e.g. `concepts`) are used, ensuring compatibility
 * with legacy toolchains.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <cstdint>

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @name Encoding pipeline dimensions
 * @{
 *
 * These constants define the fixed dimensions of the mars2grib encoding pipeline.
 *
 * They are deliberately defined as `inline constexpr` values rather than enums,
 * because they are iterated over at compile time using index sequences.
 *
 * Changing these values directly affects:
 * - the size of all generated dispatch tables,
 * - the number of compile-time instantiations.
 */
inline constexpr std::size_t NUM_STAGES   = 3;
inline constexpr std::size_t NUM_SECTIONS = 6;
/** @} */


/**
 * @name Encoding stages
 * @{
 *
 * Logical stages of the encoding pipeline.
 *
 * Each concept may participate in zero or more stages, as determined by its
 * compile-time applicability predicate.
 */
inline constexpr std::size_t StageAllocate = 0;  ///< Structure allocation stage
inline constexpr std::size_t StagePreset   = 1;  ///< Metadata preset stage
inline constexpr std::size_t StageRuntime  = 2;  ///< Runtime-dependent encoding
/** @} */


/**
 * @name GRIB2 sections
 * @{
 *
 * Numeric identifiers for GRIB2 sections, aligned with the GRIB2 specification:
 * https://codes.ecmwf.int/grib/format/grib2/sections/
 *
 * These values are used as compile-time indices into concept dispatch tables.
 */
inline constexpr std::size_t SecIndicatorSection          = 0;
inline constexpr std::size_t SecIdentificationSection     = 1;
inline constexpr std::size_t SecLocalUseSection           = 2;
inline constexpr std::size_t SecGridDefinitionSection     = 3;
inline constexpr std::size_t SecProductDefinitionSection  = 4;
inline constexpr std::size_t SecDataRepresentationSection = 5;
/** @} */


/**
 * @brief Canonical function pointer type for concept operations.
 *
 * Each entry in a concept dispatch table is a pointer to a fully specialized
 * concept operation, instantiated for:
 * - a fixed encoding stage,
 * - a fixed GRIB section,
 * - a fixed concept variant.
 *
 * The signature is uniform across all concepts.
 */
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using Fn = void (*)(const MarsDict_t&, const GeoDict_t&, const ParDict_t&, const OptDict_t&, OutDict_t&);


/**
 * @brief Compile-time list of values.
 *
 * A lightweight type-level container used to represent a list of compile-time
 * constants (typically enum values).
 *
 * This replaces runtime containers and enables static iteration via templates.
 */
template <auto... Vals>
struct ValueList {};


/**
 * @brief Compile-time iteration over a ValueList.
 *
 * Applies a templated functor `Func<V>` to each value in the list, in order.
 *
 * This is used to:
 * - iterate over all variants of a concept,
 * - register each variant into the registry.
 *
 * @tparam List Compile-time list of values
 * @tparam Func Unary template functor with a static `apply(...)` method
 */
template <class List, template <auto> class Func>
struct ForEachValue;


/// @cond INTERNAL
// Non-empty list specialization
template <auto Head, auto... Tail, template <auto> class Func>
struct ForEachValue<ValueList<Head, Tail...>, Func> {
    template <class... Args>
    static void run(Args&&... args) {
        Func<Head>::apply(std::forward<Args>(args)...);
        ForEachValue<ValueList<Tail...>, Func>::run(std::forward<Args>(args)...);
    }
};

// Empty list specialization
template <template <auto> class Func>
struct ForEachValue<ValueList<>, Func> {
    template <class... Args>
    static void run(Args&&...) {}
};
/// @endcond


/**
 * @brief Generate one row of a concept dispatch table for a fixed stage.
 *
 * Each row corresponds to a single encoding stage and contains one function
 * pointer per GRIB section.
 *
 * @tparam ConceptInfo Concept metadata type
 * @tparam Variant     Concept variant
 * @tparam Stage       Encoding stage
 */
template <class ConceptInfo, auto Variant, class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t, std::size_t Stage, std::size_t... Secs>
constexpr std::array<Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, sizeof...(Secs)> makeStageRow(
    std::index_sequence<Secs...>) {
    return {{ConceptInfo::template entry<Stage, Secs,  // Stage, Sec
                                         Variant,      // Variant
                                         MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>()...}};
}

/**
 * @brief Generate a full dispatch table for a concept variant.
 *
 * The resulting table has dimensions:
 *
 * ```
 * [NUM_STAGES][NUM_SECTIONS]
 * ```
 *
 * Every cell contains a function pointer to a fully specialized concept
 * operation. Inapplicable entries point to a throwing stub generated by
 * `ConceptInfo::entry`.
 */
template <class ConceptInfo, auto Variant, class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t, std::size_t... Stages>
constexpr std::array<std::array<Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, NUM_SECTIONS>,
                     sizeof...(Stages)>
makeTable(std::index_sequence<Stages...>) {
    return {{makeStageRow<ConceptInfo, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t, Stages>(
        std::make_index_sequence<NUM_SECTIONS>{})...}};
}

/**
 * @brief Convenience wrapper to generate a complete concept table.
 */
template <class ConceptInfo, auto Variant, class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
constexpr auto makeConceptTable() {
    return makeTable<ConceptInfo, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>(
        std::make_index_sequence<NUM_STAGES>{});
}


/**
 * @brief Helper to register a single concept variant.
 *
 * This helper:
 * - builds the compile-time dispatch table for a given variant,
 * - inserts it into the registry under the concept and variant names.
 */
template <class Registry, class ConceptInfo, class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct RegisterVariantHelper {
    template <auto Variant>
    struct Func {
        static void apply(Registry& registry) {
            auto table =
                makeConceptTable<ConceptInfo, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>();

            registry.add(ConceptInfo::name, ConceptInfo::template variantName<Variant>(), std::move(table));
        }
    };
};


/**
 * @brief Register all variants of a concept.
 *
 * Iterates over the provided `ValueList` of variants and registers each one
 * into the registry.
 *
 * This is the final step that bridges:
 * - compile-time table generation
 * - runtime concept lookup
 */
template <class ConceptInfo, class VariantList, class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct RegisterVariants;


template <class ConceptInfo, auto... Values, class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
struct RegisterVariants<ConceptInfo, ValueList<Values...>, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> {
    template <class Registry>
    static void run(Registry& registry) {
        using Helper =
            RegisterVariantHelper<Registry, ConceptInfo, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;

        ForEachValue<ValueList<Values...>, Helper::template Func>::run(registry);
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

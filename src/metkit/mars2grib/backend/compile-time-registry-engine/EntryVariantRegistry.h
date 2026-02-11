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
/// @file EntryVariantRegistry.h
/// @brief Compile-time registry engine for concept/variant indexing and metadata.
///
/// This header implements the **core compile-time indexing machinery** used by
/// mars2grib to map *concepts* and *their variants* onto stable, contiguous
/// integer domains.
///
/// The registry is entirely **header-only** and operates exclusively at
/// compile time, producing constexpr tables that are later consumed by
/// high-performance runtime code (dispatch tables, encoding plans, etc.).
///
/// -----------------------------------------------------------------------------
/// Design overview
/// -----------------------------------------------------------------------------
///
/// The fundamental abstraction is a **TypeList of Entry descriptors**, where
/// each Entry represents:
///
/// - one semantic concept (e.g. "levelType", "timeRange")
/// - a fixed number of variants
/// - an associated enum type (`Entry::Variant`)
///
/// From this list, the registry derives:
///
/// - global variant indices (flattened space)
/// - per-concept local indices
/// - concept identifiers
/// - name lookup tables (concept names, variant names)
///
/// All tables are:
///
/// - constexpr
/// - contiguous
/// - allocation-free
/// - index-stable
///
/// -----------------------------------------------------------------------------
/// Two-stage model
/// -----------------------------------------------------------------------------
///
/// The registry is conceptually split into two stages:
///
/// - **Stage 1 (Index arithmetic)**:
/// Computes offsets, indices, and relationships between concepts and
/// variants.
///
/// - **Stage 2 (Table materialization)**:
/// Builds constexpr arrays that can be used directly at runtime.
///
/// -----------------------------------------------------------------------------
/// Constraints and invariants
/// -----------------------------------------------------------------------------
///
/// The following invariants are assumed and enforced:
///
/// - Every Entry type provides:
/// - `static constexpr std::size_t variant_count`
/// - `using Variant = <enum type>`
/// - `using VariantList = ValueList<...>`
/// - `static constexpr std::string_view entryName()`
/// - `template <auto V> static constexpr std::string_view variantName()`
///
/// - The order of Entries in the TypeList defines:
/// - concept identifiers
/// - block ordering in flattened tables
///
/// - All variant enums are disjoint by type.
///
/// Violation of these assumptions results in a compile-time error.
///
#pragma once

// system includes
#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/RegisterEntryDescriptor.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/utils.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::compile_time_registry_engine {

///
/// @namespace detail
/// @brief Internal implementation details of the compile-time registry engine.
///
/// This namespace contains **low-level meta-programming utilities** that are
/// not part of the public API and must not be relied upon directly by users.
///
/// All entities in this namespace are subject to change without notice.
///
namespace detail {


// =================================================================================================
// 0) Helper: dependent_false
// =================================================================================================

///
/// @brief Helper metafunction to trigger dependent compile-time errors.
///
/// `dependent_false<T...>` always evaluates to `false`, but is dependent on
/// its template parameters. This allows `static_assert` to be placed in
/// templates without causing immediate hard errors during parsing.
///
/// Typical use:
/// - provide meaningful diagnostics in impossible template branches
///
template <typename...>
struct dependent_false : std::false_type {};


// =================================================================================================
// 1) TypeList utilities
// =================================================================================================

///
/// @brief Compile-time index lookup of a type inside a TypeList.
///
/// `IndexOf<T, List>::value` yields the zero-based index of `T` within `List`.
///
/// @tparam T    Type to search for
/// @tparam List TypeList to scan
///
/// @note
/// The behavior is undefined if `T` does not appear in `List`.
/// Such misuse is considered a logic error in registry construction.
///
template <typename T, typename List>
struct IndexOf;

///
/// @brief Recursive case: head matches the searched type.
///
template <typename T, typename... Tail>
struct IndexOf<T, TypeList<T, Tail...>> : std::integral_constant<std::size_t, 0> {};

///
/// @brief Recursive case: head does not match, continue scanning.
///
template <typename T, typename Head, typename... Tail>
struct IndexOf<T, TypeList<Head, Tail...>>
    : std::integral_constant<std::size_t, 1 + IndexOf<T, TypeList<Tail...>>::value> {};


///
/// @brief Compute the total number of variants across all entries.
///
/// This metafunction folds over the TypeList and accumulates
/// `Entry::variant_count` for each Entry.
///
/// @tparam List TypeList of Entry descriptors
///
template <typename List>
struct TotalVariantCount;

template <>
struct TotalVariantCount<TypeList<>> : std::integral_constant<std::size_t, 0> {};

template <typename Head, typename... Tail>
struct TotalVariantCount<TypeList<Head, Tail...>>
    : std::integral_constant<std::size_t, Head::variant_count + TotalVariantCount<TypeList<Tail...>>::value> {};


// =================================================================================================
//  2) CSR-style offsets: EntryOffset<I>
// =================================================================================================

///
/// @brief Compute the global offset of the I-th Entry in the flattened variant space.
///
/// The offset is defined as the sum of `variant_count` of all preceding Entries.
///
/// This is analogous to CSR (Compressed Sparse Row) prefix sums.
///
/// @tparam I    Entry index
/// @tparam List TypeList of Entries
///

template <std::size_t I, typename List>
struct EntryOffset;

template <typename Head, typename... Tail>
struct EntryOffset<0, TypeList<Head, Tail...>> : std::integral_constant<std::size_t, 0> {};

template <std::size_t I, typename Head, typename... Tail>
struct EntryOffset<I, TypeList<Head, Tail...>>
    : std::integral_constant<std::size_t, Head::variant_count + EntryOffset<I - 1, TypeList<Tail...>>::value> {};


// =================================================================================================
//  3) Map Variant enum type -> Entry by scanning the typelist
// =================================================================================================

///
/// @brief Map a variant enum type to its owning Entry descriptor.
///
/// This metafunction scans the Entries TypeList and selects the Entry whose
/// `Entry::Variant` type exactly matches the given enum.
///
/// @tparam Enum Variant enum type
/// @tparam List Entries TypeList
///
/// @note
/// Failure to find a matching Entry results in a compile-time error.
///
template <typename Enum, typename List, typename = void>
struct EntryFromVariantEnum;

template <typename Enum>
struct EntryFromVariantEnum<Enum, TypeList<>, void> {
    static_assert(dependent_false<Enum>::value,
                  "Variant enum type not associated to any entry in EntriesList. "
                  "Check: (a) entry is in the TypeList, (b) Entry::Variant matches exactly.");
    using type = void;
};

template <typename Enum, typename Head, typename... Tail>
struct EntryFromVariantEnum<Enum, TypeList<Head, Tail...>,
                            std::enable_if_t<std::is_same_v<Enum, typename Head::Variant>>> {
    using type = Head;
};

template <typename Enum, typename Head, typename... Tail>
struct EntryFromVariantEnum<Enum, TypeList<Head, Tail...>,
                            std::enable_if_t<!std::is_same_v<Enum, typename Head::Variant>>> {
    using type = typename EntryFromVariantEnum<Enum, TypeList<Tail...>>::type;
};


// =================================================================================================
//  4) VariantList-driven local index (constexpr search)
// =================================================================================================

///
/// @brief Compute the local index of a variant within its owning concept.
///
/// This is a constexpr linear search over a ValueList.
///
/// @param v   Variant enum value
/// @return   Local index, or `-1` cast to `std::size_t` if not found
///
/// @note
/// Returning `-1` is intentional; callers translate this into the `missing`
/// sentinel.
///
template <typename Enum>
constexpr std::size_t value_index_impl(Enum, std::size_t, ValueList<>) {
    return static_cast<std::size_t>(-1);
}

template <typename Enum, auto Head, auto... Tail>
constexpr std::size_t value_index_impl(Enum v, std::size_t pos, ValueList<Head, Tail...>) {
    return (v == static_cast<Enum>(Head)) ? pos : value_index_impl<Enum>(v, pos + 1, ValueList<Tail...>{});
}

template <typename Enum, typename VariantListT>
constexpr std::size_t value_index(Enum v, VariantListT) {
    return value_index_impl<Enum>(v, 0, VariantListT{});
}


// =================================================================================================
//  6) Per-entry blocks for Stage2
//     (NOTE: ConceptsT is passed explicitly => no "Concepts" free name)
// =================================================================================================

///
/// @brief Generate a block of repeated concept identifiers.
///
/// Each concept contributes a contiguous block of size `variant_count`,
/// where every element equals the concept identifier.
///
/// This is used to build the flattened concept-id table.
///
template <std::size_t ConceptIndex, typename Concept, std::size_t... Is>
constexpr std::array<std::size_t, Concept::variant_count> concept_id_block_impl(std::index_sequence<Is...>) {
    return {{(static_cast<void>(Is), ConceptIndex)...}};
}

template <std::size_t ConceptIndex, typename Concept>
constexpr std::array<std::size_t, Concept::variant_count> concept_id_block() {
    return concept_id_block_impl<ConceptIndex, Concept>(std::make_index_sequence<Concept::variant_count>{});
}

template <typename Concept, std::size_t... Is>
constexpr std::array<std::size_t, Concept::variant_count> variant_id_block_impl(std::index_sequence<Is...>) {
    return {{Is...}};
}

template <typename Concept>
constexpr std::array<std::size_t, Concept::variant_count> variant_id_block() {
    return variant_id_block_impl<Concept>(std::make_index_sequence<Concept::variant_count>{});
}

template <typename Concept, std::size_t... Is>
constexpr std::array<std::string_view, Concept::variant_count> concept_name_block_impl(std::index_sequence<Is...>) {
    return {{(static_cast<void>(Is), Concept::entryName())...}};
}

template <typename Concept>
constexpr std::array<std::string_view, Concept::variant_count> concept_name_block() {
    return concept_name_block_impl<Concept>(std::make_index_sequence<Concept::variant_count>{});
}

template <typename Concept, auto... Vs>
constexpr std::array<std::string_view, sizeof...(Vs)> variant_name_block_impl(ValueList<Vs...>) {
    return {{Concept::template variantName<Vs>()...}};
}

template <typename Concept>
constexpr auto variant_name_block() {
    return variant_name_block_impl<Concept>(typename Concept::VariantList{});
}


// =================================================================================================
//  7) Build full tables by recursion over ConceptsT
// =================================================================================================

///
/// @brief Build full registry tables by recursion over the Entries TypeList.
///
/// Each builder recursively concatenates per-entry blocks to form a
/// flattened table aligned with the global variant index space.
///
/// All builders terminate on `TypeList<>`.
///

template <typename List, std::size_t BaseIndex>
struct BuildConceptIdTableWithBase;

template <std::size_t BaseIndex>
struct BuildConceptIdTableWithBase<TypeList<>, BaseIndex> {
    static constexpr std::array<std::size_t, 0> value() { return {}; }
};

template <typename Head, typename... Tail, std::size_t BaseIndex>
struct BuildConceptIdTableWithBase<TypeList<Head, Tail...>, BaseIndex> {
    static constexpr auto value() {
        return concat(concept_id_block<BaseIndex, Head>(),
                      BuildConceptIdTableWithBase<TypeList<Tail...>, BaseIndex + 1>::value());
    }
};

template <typename List>
struct BuildVariantIdTable;

template <>
struct BuildVariantIdTable<TypeList<>> {
    static constexpr std::array<std::size_t, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildVariantIdTable<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(variant_id_block<Head>(), BuildVariantIdTable<TypeList<Tail...>>::value());
    }
};


template <typename List>
struct BuildConceptNameTable;

template <>
struct BuildConceptNameTable<TypeList<>> {
    static constexpr std::array<std::string_view, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildConceptNameTable<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(concept_name_block<Head>(), BuildConceptNameTable<TypeList<Tail...>>::value());
    }
};


template <typename List>
struct BuildVariantNameTable;

template <>
struct BuildVariantNameTable<TypeList<>> {
    static constexpr std::array<std::string_view, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildVariantNameTable<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(variant_name_block<Head>(), BuildVariantNameTable<TypeList<Tail...>>::value());
    }
};


template <typename List>
struct BuildConceptNames;

template <>
struct BuildConceptNames<TypeList<>> {
    static constexpr std::array<std::string_view, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildConceptNames<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(std::array<std::string_view, 1>{{Head::entryName()}},
                      BuildConceptNames<TypeList<Tail...>>::value());
    }
};


template <typename List>
struct BuildConceptOffsetsTable;

template <>
struct BuildConceptOffsetsTable<TypeList<>> {
    static constexpr std::array<std::size_t, 1> value() { return {{0}}; }
};

template <typename Head, typename... Tail>
struct BuildConceptOffsetsTable<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        constexpr auto tail = BuildConceptOffsetsTable<TypeList<Tail...>>::value();

        std::array<std::size_t, tail.size() + 1> result{};
        result[0] = 0;

        for (std::size_t i = 0; i < tail.size(); ++i) {
            result[i + 1] = tail[i] + Head::variant_count;
        }
        return result;
    }
};

}  // namespace detail


// =================================================================================================
//  8) EntryVariantRegistry
//     - Same public API names as your original VariantRegistry
//     - ConceptId is the derived concept number (IndexOf<Concept, Concepts>::value)
// =================================================================================================

///
/// @brief Compile-time registry providing concept/variant indexing and metadata.
///
/// `EntryVariantRegistry` exposes a **stable public API** for:
///
/// - computing global and local variant indices
/// - retrieving concept identifiers
/// - mapping enums and strings to indices
/// - accessing constexpr metadata tables
///
/// @tparam EntriesListT TypeList of Entry descriptors
///
/// This type is:
/// - stateless
/// - constexpr-friendly
/// - safe to use in headers
///
template <typename EntriesListT>
struct EntryVariantRegistry {

    using Concepts = EntriesListT;

    ///
    /// @name Registry dimensions and sentinels
    /// @{
    ///
    /// These constants define:
    /// - sentinel values for error handling
    /// - fixed structural dimensions of the encoding pipeline
    ///
    static constexpr std::size_t not_applicable = NOT_APPLICABLE;
    static constexpr std::size_t invalid        = INVALID;
    static constexpr std::size_t missing        = MISSING;
    static constexpr std::size_t NSections      = NUM_SECTIONS;
    static constexpr std::size_t NStages        = NUM_STAGES;

    static constexpr std::size_t NConcepts = Concepts::size;
    static constexpr std::size_t NVariants = detail::TotalVariantCount<Concepts>::value;
    /// @}


    // ---- STAGE 1 ----

    static constexpr std::size_t numberOfVariants() { return NVariants; }


    template <typename Concept>
    static constexpr std::size_t offset() {
        return detail::EntryOffset<detail::IndexOf<Concept, Concepts>::value, Concepts>::value;
    }

    template <typename Enum>
    static constexpr std::size_t offset_from_enum_type() {
        using Concept = typename detail::EntryFromVariantEnum<Enum, Concepts>::type;
        return offset<Concept>();
    }

    template <typename Enum>
    static constexpr std::size_t offset(Enum) {
        return offset_from_enum_type<Enum>();
    }

    template <typename Enum>
    static constexpr std::size_t conceptId(Enum) {
        using Concept = typename detail::EntryFromVariantEnum<Enum, Concepts>::type;
        return detail::IndexOf<Concept, Concepts>::value;
    }

    template <typename Enum>
    static constexpr std::size_t localIndex(Enum v) {
        using Concept         = typename detail::EntryFromVariantEnum<Enum, Concepts>::type;
        constexpr auto list   = typename Concept::VariantList{};
        const std::size_t idx = detail::value_index<Enum>(v, list);
        return (idx == static_cast<std::size_t>(-1)) ? missing : idx;
    }

    template <typename Enum>
    static constexpr std::size_t globalIndex(Enum v) {
        using Concept        = typename detail::EntryFromVariantEnum<Enum, Concepts>::type;
        const std::size_t li = localIndex(v);
        return (li == missing) ? missing : (offset<Concept>() + li);
    }


    // ---- STAGE 2 ----

    ///
    /// @brief Precomputed constexpr lookup tables.
    ///
    /// These arrays are indexed by **global variant index** and provide
    /// O(1) access to:
    ///
    /// - concept identifiers (variantId -> conceptId)
    /// - local variant indices (variantId -> localIndex)
    /// - concept names (variantId -> conceptName)
    /// - variant names (variantId -> variantName)
    /// - concept name lookup (conceptId -> conceptName)
    ///
    /// They are safe to expose as `inline constexpr` objects.
    ///
    static constexpr auto conceptIdTable() { return detail::BuildConceptIdTableWithBase<Concepts, 0>::value(); }
    static constexpr auto variantIdTable() { return detail::BuildVariantIdTable<Concepts>::value(); }
    static constexpr auto conceptNameTable() { return detail::BuildConceptNameTable<Concepts>::value(); }
    static constexpr auto variantNameTable() { return detail::BuildVariantNameTable<Concepts>::value(); }
    static constexpr auto conceptNamesTable() { return detail::BuildConceptNames<Concepts>::value(); }

    ///
    /// @brief Concept offset table (CSR-style).
    ///
    /// This array has size `NConcepts + 1` and defines half-open
    /// global variant index ranges for each concept:
    ///
    /// \code
    /// concept i → [ offsets[i], offsets[i+1] )
    /// \endcode
    ///
    /// The last entry equals `NVariants`.
    ///
    static constexpr auto conceptOffsetsTable() { return detail::BuildConceptOffsetsTable<Concepts>::value(); }

    static inline constexpr auto conceptIdArr   = conceptIdTable();
    static inline constexpr auto variantIdArr   = variantIdTable();
    static inline constexpr auto conceptNameArr = conceptNameTable();
    static inline constexpr auto variantNameArr = variantNameTable();
    static inline constexpr auto conceptNames   = conceptNamesTable();
    static inline constexpr auto conceptOffsets = conceptOffsetsTable();

private:

    template <typename Concept, std::size_t... Is>
    static constexpr std::array<std::size_t, Concept::variant_count> make_id_array_from_concept_impl(
        std::index_sequence<Is...>) {
        // VariantList order => contiguous [offset .. offset+variant_count)
        constexpr std::size_t off = offset<Concept>();
        return {{(static_cast<void>(Is), off + Is)...}};
    }

public:

    template <typename Concept>
    static constexpr std::array<std::size_t, Concept::variant_count> make_id_array_from_concept() {
        return make_id_array_from_concept_impl<Concept>(std::make_index_sequence<Concept::variant_count>{});
    }

    template <typename Concept, auto... Vs>
    static constexpr std::array<std::size_t, sizeof...(Vs)> make_id_array_from_variants() {
        using Enum = typename Concept::Variant;
        static_assert((std::is_same_v<Enum, decltype(Vs)> && ...),
                      "All variant values must belong to Concept::Variant");
        return {{globalIndex(static_cast<Enum>(Vs))...}};
    }

    // ---- STAGE 3 ----

    ///
    /// @brief Runtime utilities for string-based lookup.
    ///
    /// These functions provide a bridge between dynamic inputs (strings)
    /// and the compile-time registry.
    ///
    /// They are intentionally linear-time and intended for:
    /// - diagnostics
    /// - configuration parsing
    /// - debugging paths
    ///
    /// They must not be used in hot paths.
    ///
    static std::size_t conceptId(const std::string& name) {
        for (std::size_t i = 0; i < NConcepts; ++i) {
            if (conceptNames[i] == name) {
                return i;
            }
        }
        return missing;
    }

    static std::size_t localIndex(const std::string& conceptName, const std::string& variantName) {
        const std::size_t cid = conceptId(conceptName);
        if (cid == missing)
            return missing;

        const std::size_t begin = offset(cid);
        const std::size_t end   = (cid + 1 < NConcepts) ? offset(cid + 1) : NVariants;
        for (std::size_t i = begin; i < end; ++i) {
            if (variantNameArr[i] == variantName) {
                return variantIdArr[i];  // local index
            }
        }
        return missing;
    }

    static std::size_t globalIndex(const std::string& conceptName, const std::string& variantName) {
        const std::size_t cid = conceptId(conceptName);
        if (cid == missing)
            return missing;

        const std::size_t begin = offset(cid);
        const std::size_t end   = (cid + 1 < NConcepts) ? offset(cid + 1) : NVariants;

        for (std::size_t i = begin; i < end; ++i) {
            if (variantNameArr[i] == variantName) {
                return i;
            }
        }
        return missing;
    }
};

}  // namespace metkit::mars2grib::backend::compile_time_registry_engine

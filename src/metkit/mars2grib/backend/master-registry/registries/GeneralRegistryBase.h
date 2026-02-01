#pragma once

/**
 * @file GeneralRegistryBase.h
 * @brief Base class for compact registries with name and index lookup.
 *
 * This header provides `GeneralRegistryBase`, a reusable, entity-agnostic
 * base that implements:
 *
 * - CSR-style compact indexing (owner -> value block)
 * - total element count
 * - materialized lookup tables:
 *   - globalIndex -> ownerId
 *   - globalIndex -> localId
 *   - globalIndex -> ownerName
 *   - globalIndex -> valueName
 * - runtime name lookup by linear search:
 *   - ownerId = getOwnerId("ownerName")
 *   - localId = getLocalId("ownerName", "valueName")
 *   - globalId = getGlobalId("ownerName", "valueName")
 *
 * The registry assumes that each owner contributes a contiguous block
 * of values in a global space (CSR layout).
 *
 * @par Contract for Owner types
 * Each owner type in `OwnersT` must define:
 * - `static constexpr std::size_t id;`
 * - `static constexpr std::size_t value_count;`
 * - `using EnumType = <enum type>;`  (0-based contiguous, compact)
 * - `static constexpr std::string_view ownerName();`
 * - `template <EnumType V> static constexpr std::string_view valueName();`
 *
 * @par Contract for OwnersT
 * `OwnersT` must be `typelist::TypeList<...>` and provide:
 * - `static constexpr std::size_t size;`
 *
 * @ingroup master_registry_registries
 */

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "backend/master-registry/arrays/ArrayBuilders.h"
#include "backend/master-registry/arrays/ArrayConcat.h"
#include "backend/master-registry/indexing/CompactIndexing.h"
#include "backend/master-registry/indexing/OffsetComputation.h"
#include "backend/master-registry/typelist/TypeList.h"

namespace metkit::mars2grib::backend::master_registry::registries {

namespace detail {

using namespace metkit::mars2grib::backend::master_registry::typelist;
using namespace metkit::mars2grib::backend::master_registry::arrays;
using namespace metkit::mars2grib::backend::master_registry::indexing;

// ---------------------------------------------------------------------------
// Per-owner blocks (materialization primitives)
// ---------------------------------------------------------------------------

template <typename Owner, std::size_t... Is>
constexpr std::array<std::size_t, Owner::value_count> ownerIdBlockImpl(std::index_sequence<Is...>) {
    return {{(static_cast<void>(Is), static_cast<std::size_t>(Owner::id))...}};
}

template <typename Owner>
constexpr std::array<std::size_t, Owner::value_count> ownerIdBlock() {
    return ownerIdBlockImpl<Owner>(std::make_index_sequence<Owner::value_count>{});
}

template <typename Owner>
constexpr std::array<std::size_t, Owner::value_count> localIdBlock() {
    return iotaArray<std::size_t, Owner::value_count>();
}

template <typename Owner, std::size_t... Is>
constexpr std::array<std::string_view, Owner::value_count> ownerNameBlockImpl(std::index_sequence<Is...>) {
    return {{(static_cast<void>(Is), Owner::ownerName())...}};
}

template <typename Owner>
constexpr std::array<std::string_view, Owner::value_count> ownerNameBlock() {
    return ownerNameBlockImpl<Owner>(std::make_index_sequence<Owner::value_count>{});
}

template <typename Owner, std::size_t... Is>
constexpr std::array<std::string_view, Owner::value_count> valueNameBlockImpl(std::index_sequence<Is...>) {
    using E = typename Owner::EnumType;
    return {{Owner::template valueName<static_cast<E>(Is)>()...}};
}

template <typename Owner>
constexpr std::array<std::string_view, Owner::value_count> valueNameBlock() {
    return valueNameBlockImpl<Owner>(std::make_index_sequence<Owner::value_count>{});
}

// ---------------------------------------------------------------------------
// Per-owner summary tables (ownerIndex -> name/id/count/offset)
// ---------------------------------------------------------------------------

template <typename Owner>
constexpr std::array<std::string_view, 1> ownerNameScalar() {
    return {{Owner::ownerName()}};
}

template <typename Owner>
constexpr std::array<std::size_t, 1> ownerIdScalar() {
    return {{static_cast<std::size_t>(Owner::id)}};
}

template <typename Owner>
constexpr std::array<std::size_t, 1> ownerCountScalar() {
    return {{static_cast<std::size_t>(Owner::value_count)}};
}

// ---------------------------------------------------------------------------
// Typelist recursion helpers to build arrays (no external traversal utilities)
// ---------------------------------------------------------------------------

template <typename List>
struct BuildOwnerNameByOwner;

template <>
struct BuildOwnerNameByOwner<TypeList<>> {
    static constexpr std::array<std::string_view, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildOwnerNameByOwner<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(ownerNameScalar<Head>(), BuildOwnerNameByOwner<TypeList<Tail...>>::value());
    }
};

template <typename List>
struct BuildOwnerIdByOwner;

template <>
struct BuildOwnerIdByOwner<TypeList<>> {
    static constexpr std::array<std::size_t, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildOwnerIdByOwner<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(ownerIdScalar<Head>(), BuildOwnerIdByOwner<TypeList<Tail...>>::value());
    }
};

template <typename List>
struct BuildCountByOwner;

template <>
struct BuildCountByOwner<TypeList<>> {
    static constexpr std::array<std::size_t, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildCountByOwner<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(ownerCountScalar<Head>(), BuildCountByOwner<TypeList<Tail...>>::value());
    }
};

// Global tables sized by total values

template <typename List>
struct BuildOwnerIdByGlobal;

template <>
struct BuildOwnerIdByGlobal<TypeList<>> {
    static constexpr std::array<std::size_t, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildOwnerIdByGlobal<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(ownerIdBlock<Head>(), BuildOwnerIdByGlobal<TypeList<Tail...>>::value());
    }
};

template <typename List>
struct BuildLocalIdByGlobal;

template <>
struct BuildLocalIdByGlobal<TypeList<>> {
    static constexpr std::array<std::size_t, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildLocalIdByGlobal<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(localIdBlock<Head>(), BuildLocalIdByGlobal<TypeList<Tail...>>::value());
    }
};

template <typename List>
struct BuildOwnerNameByGlobal;

template <>
struct BuildOwnerNameByGlobal<TypeList<>> {
    static constexpr std::array<std::string_view, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildOwnerNameByGlobal<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(ownerNameBlock<Head>(), BuildOwnerNameByGlobal<TypeList<Tail...>>::value());
    }
};

template <typename List>
struct BuildValueNameByGlobal;

template <>
struct BuildValueNameByGlobal<TypeList<>> {
    static constexpr std::array<std::string_view, 0> value() { return {}; }
};

template <typename Head, typename... Tail>
struct BuildValueNameByGlobal<TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(valueNameBlock<Head>(), BuildValueNameByGlobal<TypeList<Tail...>>::value());
    }
};

}  // namespace detail

/**
 * @brief Base implementation of a compact owner/value registry.
 *
 * @tparam OwnersT `typelist::TypeList<...>` of owners.
 */
template <typename OwnersT>
struct GeneralRegistryBase {

    using Owners = OwnersT;

    static constexpr std::size_t NOwners = OwnersT::size;

    // ------------------------------------------------------------------------
    // Owner-level tables (size = NOwners)
    // ------------------------------------------------------------------------

    /// ownerIndex -> ownerName
    static inline constexpr auto ownerNameArr = detail::BuildOwnerNameByOwner<OwnersT>::value();

    /// ownerIndex -> ownerId (semantic id defined by owner type)
    static inline constexpr auto ownerIdArr = detail::BuildOwnerIdByOwner<OwnersT>::value();

    /// ownerIndex -> value_count
    static inline constexpr auto valueCountArr = detail::BuildCountByOwner<OwnersT>::value();

    /// ownerIndex -> CSR offset into global space
    static inline constexpr auto offsetArr =
        metkit::mars2grib::backend::master_registry::indexing::computeOffsets<NOwners>(valueCountArr);

    /// Total number of values in global space
    static constexpr std::size_t NValues =
        metkit::mars2grib::backend::master_registry::indexing::computeTotalCount<NOwners>(valueCountArr);

    // ------------------------------------------------------------------------
    // Global tables (size = NValues)
    // ------------------------------------------------------------------------

    /// globalIndex -> ownerId
    static inline constexpr auto ownerIdByGlobalArr = detail::BuildOwnerIdByGlobal<OwnersT>::value();

    /// globalIndex -> localId (0..value_count-1 within owner block)
    static inline constexpr auto localIdByGlobalArr = detail::BuildLocalIdByGlobal<OwnersT>::value();

    /// globalIndex -> ownerName
    static inline constexpr auto ownerNameByGlobalArr = detail::BuildOwnerNameByGlobal<OwnersT>::value();

    /// globalIndex -> valueName
    static inline constexpr auto valueNameByGlobalArr = detail::BuildValueNameByGlobal<OwnersT>::value();

    // ------------------------------------------------------------------------
    // Indexing primitives (value-level; assumes compact enums)
    // ------------------------------------------------------------------------

    /**
     * @brief Compute global index from ownerIndex and localIndex.
     */
    static constexpr std::size_t globalIndex(std::size_t ownerIndex, std::size_t localIndex) noexcept {
        return metkit::mars2grib::backend::master_registry::indexing::makeGlobalIndex(offsetArr[ownerIndex],
                                                                                      localIndex);
    }

    /**
     * @brief Get owner index from an owner name (runtime).
     *
     * @throws std::invalid_argument if the name is unknown.
     */
    static std::size_t getOwnerIndex(std::string_view ownerName) {
        for (std::size_t i = 0; i < NOwners; ++i) {
            if (ownerNameArr[i] == ownerName)
                return i;
        }
        throw std::invalid_argument("GeneralRegistryBase::getOwnerIndex: unknown owner name");
    }

    /**
     * @brief Get owner semantic id from an owner name (runtime).
     *
     * @throws std::invalid_argument if the name is unknown.
     */
    static std::size_t getOwnerId(std::string_view ownerName) { return ownerIdArr[getOwnerIndex(ownerName)]; }

    /**
     * @brief Get local id from (ownerName, valueName) (runtime).
     *
     * @throws std::invalid_argument if unknown.
     */
    static std::size_t getLocalId(std::string_view ownerName, std::string_view valueName) {
        const auto oi    = getOwnerIndex(ownerName);
        const auto begin = offsetArr[oi];
        const auto end   = begin + valueCountArr[oi];

        for (std::size_t gi = begin; gi < end; ++gi) {
            if (valueNameByGlobalArr[gi] == valueName) {
                return localIdByGlobalArr[gi];
            }
        }
        throw std::invalid_argument("GeneralRegistryBase::getLocalId: unknown value name for owner");
    }

    /**
     * @brief Get global id from (ownerName, valueName) (runtime).
     *
     * @throws std::invalid_argument if unknown.
     */
    static std::size_t getGlobalId(std::string_view ownerName, std::string_view valueName) {
        const auto oi    = getOwnerIndex(ownerName);
        const auto begin = offsetArr[oi];
        const auto end   = begin + valueCountArr[oi];

        for (std::size_t gi = begin; gi < end; ++gi) {
            if (valueNameByGlobalArr[gi] == valueName) {
                return gi;
            }
        }
        throw std::invalid_argument("GeneralRegistryBase::getGlobalId: unknown value name for owner");
    }
};

}  // namespace metkit::mars2grib::backend::master_registry::registries

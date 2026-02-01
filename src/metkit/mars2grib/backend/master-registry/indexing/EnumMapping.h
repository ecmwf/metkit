#pragma once

/**
 * @file EnumMapping.h
 * @brief Compile-time mapping between enum types and owner types.
 *
 * This header provides utilities to map an enum type to its owning
 * type (e.g. Concept::Variant → Concept).
 *
 * It is used by registries to:
 * - infer ownership from enum types
 * - enforce correct enum–owner associations at compile time
 *
 * The mapping is *type-based*, not value-based.
 *
 * @ingroup master_registry_indexing
 */

#include <type_traits>

#include "backend/master-registry/typelist/TypeList.h"
#include "backend/master-registry/utilities/DependentFalse.h"

namespace metkit::mars2grib::backend::master_registry::indexing {

using namespace metkit::mars2grib::backend::master_registry::typelist;

/**
 * @brief Map an enum type to its owning type by scanning a typelist.
 *
 * This metafunction finds the first type `Owner` in `Owners`
 * such that:
 *
 * \code
 * std::is_same_v<Enum, typename Owner::EnumType>
 * \endcode
 *
 * @tparam Enum Enum type to map.
 * @tparam Owners Typelist of possible owners.
 * @tparam Enable SFINAE helper.
 */
template <typename Enum, typename Owners, typename Enable = void>
struct OwnerFromEnum;

/**
 * @brief Failure case: enum not associated with any owner.
 *
 * Triggers a dependent static assertion with a clear diagnostic.
 */
template <typename Enum>
struct OwnerFromEnum<Enum, TypeList<>, void> {
    static_assert(dependent_false<Enum>::value, "Enum type is not associated with any owner in the registry");
};

/**
 * @brief Match case: owner exposes Enum as its enum type.
 *
 * Requires the owner to define:
 * \code
 * using EnumType = ...
 * \endcode
 */
template <typename Enum, typename Head, typename... Tail>
struct OwnerFromEnum<Enum, TypeList<Head, Tail...>, std::enable_if_t<std::is_same_v<Enum, typename Head::EnumType>>> {
    using type = Head;
};

/**
 * @brief Recursive case: continue scanning remaining owners.
 */
template <typename Enum, typename Head, typename... Tail>
struct OwnerFromEnum<Enum, TypeList<Head, Tail...>, std::enable_if_t<!std::is_same_v<Enum, typename Head::EnumType>>> {
    using type = typename OwnerFromEnum<Enum, TypeList<Tail...>>::type;
};

}  // namespace metkit::mars2grib::backend::master_registry::indexing

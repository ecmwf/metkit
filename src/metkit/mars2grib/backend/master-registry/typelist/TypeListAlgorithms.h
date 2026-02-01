#pragma once

/**
 * @file TypeListAlgorithms.h
 * @brief Generic compile-time algorithms operating on TypeList.
 *
 * This header provides pure type-level algorithms for `TypeList`,
 * including:
 * - type lookup
 * - containment checks
 * - transformations
 *
 * These utilities operate only on types and do not materialize
 * any runtime values.
 *
 * @ingroup master_registry_typelist
 */

#include <cstddef>
#include <type_traits>

#include "TypeList.h"

namespace metkit::mars2grib::backend::master_registry::typelist {

/**
 * @brief Helper type for dependent static assertions.
 *
 * Used to trigger meaningful compile-time errors in dependent contexts.
 *
 * @tparam Ts Dummy parameter pack to enforce dependency.
 */
template <typename... Ts>
struct dependent_false : std::false_type {};


// ============================================================================
// IndexOf
// ============================================================================

/**
 * @brief Compute the index of type `T` inside a `TypeList`.
 *
 * `IndexOf<T, TypeList<...>>::value` yields the 0-based index of `T`
 * within the list.
 *
 * @tparam T Type to locate.
 * @tparam List A `TypeList` containing `T`.
 *
 * @warning
 * If `T` is not present in `List`, compilation will fail.
 */
template <typename T, typename List>
struct IndexOf;

/**
 * @brief Base case: `T` is the first element.
 */
template <typename T, typename... Tail>
struct IndexOf<T, TypeList<T, Tail...>> : std::integral_constant<std::size_t, 0> {};

/**
 * @brief Recursive case: advance through the list.
 */
template <typename T, typename Head, typename... Tail>
struct IndexOf<T, TypeList<Head, Tail...>>
    : std::integral_constant<std::size_t, 1 + IndexOf<T, TypeList<Tail...>>::value> {};


// ============================================================================
// Contains
// ============================================================================

/**
 * @brief Check whether a type is present in a TypeList.
 *
 * @tparam T Type to check.
 * @tparam List A `TypeList`.
 */
template <typename T, typename List>
struct Contains;

/**
 * @brief Base case: empty list.
 */
template <typename T>
struct Contains<T, TypeList<>> : std::false_type {};

/**
 * @brief Match case.
 */
template <typename T, typename... Tail>
struct Contains<T, TypeList<T, Tail...>> : std::true_type {};

/**
 * @brief Recursive case.
 */
template <typename T, typename Head, typename... Tail>
struct Contains<T, TypeList<Head, Tail...>> : Contains<T, TypeList<Tail...>> {};


// ============================================================================
// Transform
// ============================================================================

/**
 * @brief Transform each type in a TypeList using a metafunction.
 *
 * @tparam List Input TypeList.
 * @tparam MetaFun Unary metafunction: `MetaFun<T>::type`.
 */
template <typename List, template <typename> class MetaFun>
struct Transform;

/**
 * @brief Transform specialization.
 */
template <template <typename> class MetaFun, typename... Ts>
struct Transform<TypeList<Ts...>, MetaFun> {
    using type = TypeList<typename MetaFun<Ts>::type...>;
};

}  // namespace metkit::mars2grib::backend::master_registry::typelist

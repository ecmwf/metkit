#pragma once

/**
 * @file TypeListTraversal.h
 * @brief Compile-time traversal utilities bridging types to values.
 *
 * This header provides utilities to *traverse* a TypeList and
 * produce compile-time values (typically std::array).
 *
 * Unlike TypeListAlgorithms.h, which operates purely at the type level,
 * this file bridges typelists to constexpr value materialization.
 *
 * @ingroup master_registry_typelist
 */

#include <array>
#include <utility>

#include "TypeList.h"

namespace metkit::mars2grib::backend::master_registry::typelist {

/**
 * @brief Traverse a TypeList and build a std::array of values.
 *
 * The provided `Builder` must expose:
 *
 * \code
 * template <typename T>
 * static constexpr ValueType make();
 * \endcode
 *
 * @tparam List Input TypeList.
 * @tparam Builder Policy defining how to build a value from a type.
 * @tparam ValueType Element type of the resulting array.
 */
template <typename List, typename Builder, typename ValueType>
struct ForEachType;

/**
 * @brief Base case: empty TypeList.
 */
template <typename Builder, typename ValueType>
struct ForEachType<TypeList<>, Builder, ValueType> {
    static constexpr std::array<ValueType, 0> value() { return {}; }
};

/**
 * @brief Recursive case: build head, then tail.
 */
template <typename Head, typename... Tail, typename Builder, typename ValueType>
struct ForEachType<TypeList<Head, Tail...>, Builder, ValueType> {

    static constexpr auto value() {
        constexpr std::array<ValueType, 1> head{{Builder::template make<Head>()}};

        constexpr auto tail = ForEachType<TypeList<Tail...>, Builder, ValueType>::value();

        return concat(head, tail);
    }
};

}  // namespace metkit::mars2grib::backend::master_registry::typelist

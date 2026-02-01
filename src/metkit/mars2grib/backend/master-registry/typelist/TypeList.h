#pragma once

/**
 * @file TypeList.h
 * @brief Minimal compile-time typelist container.
 *
 * This header defines `TypeList`, a lightweight compile-time container
 * used to hold a sequence of types.
 *
 * `TypeList` is the foundational building block for all typelist-based
 * metaprogramming in the master-registry layer. It intentionally provides
 * only structure (no algorithms).
 *
 * All algorithms operating on `TypeList` are defined in:
 * - TypeListAlgorithms.h
 * - TypeListTraversal.h
 *
 * @ingroup master_registry_typelist
 */

#include <cstddef>

namespace metkit::mars2grib::backend::master_registry::typelist {

/**
 * @brief Compile-time list of types.
 *
 * @tparam Ts Parameter pack of types.
 *
 * @note
 * `TypeList` does not impose any semantic meaning on the contained types.
 * It is purely a structural container.
 */
template <typename... Ts>
struct TypeList {

    /// Number of types in the list
    static constexpr std::size_t size = sizeof...(Ts);
};

}  // namespace metkit::mars2grib::backend::master_registry::typelist

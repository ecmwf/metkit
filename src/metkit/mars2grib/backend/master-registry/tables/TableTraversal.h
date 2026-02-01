#pragma once

/**
 * @file TableTraversal.h
 * @brief Compile-time traversal skeletons for multidimensional tables.
 *
 * This header defines generic traversal patterns used to iterate
 * over dimensions such as:
 * - owners
 * - values
 * - stages
 * - sections
 *
 * Traversal is separated from materialization and payload logic.
 *
 * @ingroup master_registry_tables
 */

#include <array>
#include <cstddef>
#include <utility>

namespace metkit::mars2grib::backend::master_registry::tables {

/**
 * @brief Traverse a 1D dimension and build an array.
 *
 * The Builder must expose:
 * \code
 * static constexpr Value make<I>();
 * \endcode
 *
 * @tparam N Size of the dimension.
 * @tparam Builder Builder policy.
 * @tparam Value Element type.
 */
template <std::size_t N, typename Builder, typename Value, std::size_t... Is>
constexpr std::array<Value, N> traverse1DImpl(std::index_sequence<Is...>) {
    return {{Builder::template make<Is>()...}};
}

/**
 * @brief Public wrapper for 1D traversal.
 */
template <std::size_t N, typename Builder, typename Value>
constexpr std::array<Value, N> traverse1D() {
    return traverse1DImpl<N, Builder, Value>(std::make_index_sequence<N>{});
}

/**
 * @brief Traverse a 2D dimension and build a nested array.
 *
 * Builder must expose:
 * \code
 * template <std::size_t I, std::size_t J>
 * static constexpr Value make();
 * \endcode
 *
 * @tparam N1 First dimension.
 * @tparam N2 Second dimension.
 * @tparam Builder Builder policy.
 * @tparam Value Element type.
 */
template <std::size_t N1, std::size_t N2, typename Builder, typename Value, std::size_t... I>
constexpr std::array<std::array<Value, N2>, N1> traverse2DImpl(std::index_sequence<I...>) {
    return {{traverse1D<N2, Builder::template Row<I>, Value>()...}};
}

/**
 * @brief Public wrapper for 2D traversal.
 */
template <std::size_t N1, std::size_t N2, typename Builder, typename Value>
constexpr std::array<std::array<Value, N2>, N1> traverse2D() {
    return traverse2DImpl<N1, N2, Builder, Value>(std::make_index_sequence<N1>{});
}

/**
 * @brief Traverse a 3D dimension and build a nested array.
 *
 * Builder must expose:
 * \code
 * template <std::size_t I>
 * struct Plane {
 *   template <std::size_t J, std::size_t K>
 *   static constexpr Value make();
 * };
 * \endcode
 *
 * @tparam N1 First dimension.
 * @tparam N2 Second dimension.
 * @tparam N3 Third dimension.
 * @tparam Builder Builder policy.
 * @tparam Value Element type.
 */
template <std::size_t N1, std::size_t N2, std::size_t N3, typename Builder, typename Value, std::size_t... I>
constexpr std::array<std::array<std::array<Value, N3>, N2>, N1> traverse3DImpl(std::index_sequence<I...>) {
    return {{traverse2D<N2, N3, typename Builder::template Plane<I>, Value>()...}};
}

/**
 * @brief Public wrapper for 3D traversal.
 */
template <std::size_t N1, std::size_t N2, std::size_t N3, typename Builder, typename Value>
constexpr std::array<std::array<std::array<Value, N3>, N2>, N1> traverse3D() {
    return traverse3DImpl<N1, N2, N3, Builder, Value>(std::make_index_sequence<N1>{});
}

}  // namespace metkit::mars2grib::backend::master_registry::tables

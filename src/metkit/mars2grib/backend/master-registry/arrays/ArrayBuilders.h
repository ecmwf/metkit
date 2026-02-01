#pragma once

/**
 * @file ArrayBuilders.h
 * @brief Reusable constexpr array construction utilities.
 *
 * This header provides a collection of small, composable
 * array builders used throughout the master-registry to
 * materialize compile-time tables.
 *
 * Typical use cases:
 * - build [0,1,2,...,N)
 * - repeat a value N times
 * - build per-owner blocks
 *
 * @ingroup master_registry_arrays
 */

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace metkit::mars2grib::backend::master_registry::arrays {

// ============================================================================
// repeatValue
// ============================================================================

/**
 * @brief Build an array filled with the same value.
 *
 * @tparam T Element type.
 * @tparam N Number of elements.
 * @tparam Is Index pack (unused, size driver).
 *
 * @param value Value to repeat.
 * @return Array of size N filled with `value`.
 */
template <typename T, std::size_t N, std::size_t... Is>
constexpr std::array<T, N> repeatValueImpl(const T& value, std::index_sequence<Is...>) {
    return {{(static_cast<void>(Is), value)...}};
}

/**
 * @brief Public wrapper for repeatValueImpl.
 */
template <typename T, std::size_t N>
constexpr std::array<T, N> repeatValue(const T& value) {
    return repeatValueImpl<T, N>(value, std::make_index_sequence<N>{});
}

// ============================================================================
// iotaArray
// ============================================================================

/**
 * @brief Build an array [0, 1, 2, ..., N-1].
 *
 * @tparam T Integer type.
 * @tparam N Number of elements.
 * @tparam Is Index pack.
 */
template <typename T, std::size_t N, std::size_t... Is>
constexpr std::array<T, N> iotaArrayImpl(std::index_sequence<Is...>) {
    return {{static_cast<T>(Is)...}};
}

/**
 * @brief Public wrapper for iotaArrayImpl.
 *
 * @tparam T Integer type.
 * @tparam N Number of elements.
 */
template <typename T, std::size_t N>
constexpr std::array<T, N> iotaArray() {
    static_assert(std::is_integral_v<T>, "iotaArray requires an integral type");
    return iotaArrayImpl<T, N>(std::make_index_sequence<N>{});
}

// ============================================================================
// enumRangeArray
// ============================================================================

/**
 * @brief Build an array of enum values assuming 0-based contiguous layout.
 *
 * Produces:
 * \code
 * { Enum(0), Enum(1), ..., Enum(N-1) }
 * \endcode
 *
 * @tparam Enum Enum type.
 * @tparam N Number of enum values.
 * @tparam Is Index pack.
 *
 * @warning
 * This assumes the enum is 0-based and contiguous.
 */
template <typename Enum, std::size_t N, std::size_t... Is>
constexpr std::array<Enum, N> enumRangeArrayImpl(std::index_sequence<Is...>) {
    return {{static_cast<Enum>(Is)...}};
}

/**
 * @brief Public wrapper for enumRangeArrayImpl.
 */
template <typename Enum, std::size_t N>
constexpr std::array<Enum, N> enumRangeArray() {
    static_assert(std::is_enum_v<Enum>, "enumRangeArray requires an enum type");
    return enumRangeArrayImpl<Enum, N>(std::make_index_sequence<N>{});
}

}  // namespace metkit::mars2grib::backend::master_registry::arrays

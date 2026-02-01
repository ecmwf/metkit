#pragma once

/**
 * @file ArrayConcat.h
 * @brief Compile-time concatenation of std::array.
 *
 * This header provides constexpr utilities to concatenate
 * two `std::array` objects into a single array.
 *
 * The implementation is index-sequence–based and does not
 * rely on loops or runtime logic.
 *
 * @ingroup master_registry_arrays
 */

#include <array>
#include <cstddef>
#include <utility>

namespace metkit::mars2grib::backend::master_registry::arrays {

/**
 * @brief Internal helper for array concatenation.
 *
 * @tparam T Element type.
 * @tparam N1 Size of first array.
 * @tparam N2 Size of second array.
 * @tparam I1 Index sequence for first array.
 * @tparam I2 Index sequence for second array.
 */
template <typename T, std::size_t N1, std::size_t N2, std::size_t... I1, std::size_t... I2>
constexpr std::array<T, N1 + N2> concatImpl(const std::array<T, N1>& a, const std::array<T, N2>& b,
                                            std::index_sequence<I1...>, std::index_sequence<I2...>) {
    return {{a[I1]..., b[I2]...}};
}

/**
 * @brief Concatenate two arrays at compile time.
 *
 * @tparam T Element type.
 * @tparam N1 Size of first array.
 * @tparam N2 Size of second array.
 *
 * @param a First array.
 * @param b Second array.
 * @return Concatenated array of size `N1 + N2`.
 */
template <typename T, std::size_t N1, std::size_t N2>
constexpr std::array<T, N1 + N2> concat(const std::array<T, N1>& a, const std::array<T, N2>& b) {
    return concatImpl<T, N1, N2>(a, b, std::make_index_sequence<N1>{}, std::make_index_sequence<N2>{});
}

}  // namespace metkit::mars2grib::backend::master_registry::arrays

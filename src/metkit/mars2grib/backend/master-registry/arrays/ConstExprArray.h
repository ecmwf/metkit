#pragma once

/**
 * @file ConstexprArray.h
 * @brief Common helpers and conventions for constexpr std::array usage.
 *
 * This header provides small, reusable utilities and conventions
 * for working with `std::array` in a constexpr / compile-time context.
 *
 * It intentionally avoids introducing new containers and instead
 * standardizes how `std::array` is constructed and manipulated
 * across the master-registry.
 *
 * @ingroup master_registry_arrays
 */

#include <array>
#include <cstddef>

namespace metkit::mars2grib::backend::master_registry::arrays {

/**
 * @brief Alias for a constexpr-friendly fixed-size array.
 *
 * This alias exists mainly for documentation and readability purposes.
 *
 * @tparam T Element type.
 * @tparam N Number of elements.
 */
template <typename T, std::size_t N>
using ConstexprArray = std::array<T, N>;

/**
 * @brief Return an empty constexpr array.
 *
 * Useful as a base case in recursive array construction.
 *
 * @tparam T Element type.
 * @return Empty array of size 0.
 */
template <typename T>
constexpr ConstexprArray<T, 0> emptyArray() {
    return {};
}

}  // namespace metkit::mars2grib::backend::master_registry::arrays

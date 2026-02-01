/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

/**
 * @file array_concat.h
 * @brief Compile-time concatenation utilities for fixed-size arrays.
 *
 * This header provides a minimal, constexpr-capable utility to **concatenate
 * two `std::array` objects into a single `std::array`**, entirely at compile time.
 *
 * -----------------------------------------------------------------------------
 * Scope and responsibility
 * -----------------------------------------------------------------------------
 *
 * This file is intentionally narrow in scope. It provides:
 *
 * - a constexpr implementation of array concatenation,
 * - suitable for use in template metaprogramming contexts,
 * - with no dynamic allocation and no runtime overhead.
 *
 * It does **not**:
 * - perform bounds checking,
 * - provide runtime utilities,
 * - introduce any registry-specific semantics.
 *
 * -----------------------------------------------------------------------------
 * Architectural role
 * -----------------------------------------------------------------------------
 *
 * This utility is a foundational building block used by higher-level
 * compile-time registry engines to:
 *
 * - assemble flattened lookup tables,
 * - concatenate per-entry or per-variant blocks,
 * - materialize large constexpr dispatch tables.
 *
 * It lives in the `detail` namespace because it is considered an
 * **implementation primitive**, not part of the public API.
 *
 * -----------------------------------------------------------------------------
 * Design constraints
 * -----------------------------------------------------------------------------
 *
 * - Fully `constexpr` (usable in constant expressions)
 * - Header-only
 * - Allocation-free
 * - Type- and size-safe
 * - Compatible with C++17
 *
 * -----------------------------------------------------------------------------
 * Complexity guarantees
 * -----------------------------------------------------------------------------
 *
 * - Compile-time complexity: O(N1 + N2)
 * - Runtime complexity: zero (fully evaluated at compile time)
 *
 * -----------------------------------------------------------------------------
 * Safety and invariants
 * -----------------------------------------------------------------------------
 *
 * - Both input arrays must have the same element type `T`.
 * - The resulting array has size `N1 + N2`.
 * - Element order is strictly preserved:
 *     - all elements of `a` precede all elements of `b`.
 */

#pragma once

// system includes
#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace metkit::mars2grib::backend::compile_time_registry_engine::detail{

/**
 * @brief Implementation helper for compile-time array concatenation.
 *
 * This function performs the actual concatenation by expanding two
 * `std::index_sequence`s corresponding to the indices of the input arrays.
 *
 * It is intentionally separated from the public interface to:
 * - avoid exposing index sequence details,
 * - allow clean forwarding from the simpler `concat` wrapper.
 *
 * @tparam T   Element type of the arrays
 * @tparam N1  Size of the first array
 * @tparam N2  Size of the second array
 * @tparam I1  Index pack for the first array
 * @tparam I2  Index pack for the second array
 *
 * @param[in] a   First input array
 * @param[in] b   Second input array
 * @param[in]     Index sequence for array `a`
 * @param[in]     Index sequence for array `b`
 *
 * @return
 *   A `std::array<T, N1 + N2>` containing:
 *   `{ a[0], a[1], ..., a[N1-1], b[0], b[1], ..., b[N2-1] }`
 *
 * @note
 * This function is intended to be invoked only from `concat`.
 */
template <typename T, std::size_t N1, std::size_t N2, std::size_t... I1, std::size_t... I2>
constexpr std::array<T, N1 + N2> concat_impl(const std::array<T, N1>& a, const std::array<T, N2>& b,
                                             std::index_sequence<I1...>, std::index_sequence<I2...>) {
    return {{a[I1]..., b[I2]...}};
}

/**
 * @brief Concatenate two `std::array` objects at compile time.
 *
 * This function provides a simple, user-facing interface for array
 * concatenation, hiding all index-sequence machinery.
 *
 * @tparam T   Element type of the arrays
 * @tparam N1  Size of the first array
 * @tparam N2  Size of the second array
 *
 * @param[in] a   First input array
 * @param[in] b   Second input array
 *
 * @return
 *   A `std::array<T, N1 + N2>` containing the elements of `a`
 *   followed by the elements of `b`.
 *
 * @note
 * This function is `constexpr` and may be used in:
 * - compile-time table construction,
 * - static data member initialization,
 * - other constant-expression contexts.
 */
template <typename T, std::size_t N1, std::size_t N2>
constexpr std::array<T, N1 + N2> concat(const std::array<T, N1>& a, const std::array<T, N2>& b) {
    return concat_impl<T, N1, N2>(a, b, std::make_index_sequence<N1>{}, std::make_index_sequence<N2>{});
}

}
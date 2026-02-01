#pragma once

/**
 * @file OffsetComputation.h
 * @brief Compile-time computation of counts and CSR-style offsets.
 *
 * This header provides generic utilities to compute:
 * - per-owner element counts
 * - prefix-sum offsets (CSR layout)
 *
 * It is intentionally independent of:
 * - enums
 * - typelist traversal
 * - semantic meaning of owners
 *
 * All inputs are provided as constexpr arrays or compile-time values.
 *
 * @ingroup master_registry_indexing
 */

#include <array>
#include <cstddef>
#include <utility>

namespace metkit::mars2grib::backend::master_registry::indexing {

/**
 * @brief Compute CSR-style offsets from a count array.
 *
 * Given:
 * \code
 * counts = [c0, c1, c2, ...]
 * \endcode
 *
 * This produces:
 * \code
 * offsets = [0, c0, c0+c1, c0+c1+c2, ...]
 * \endcode
 *
 * @tparam N Number of owners.
 * @param counts Per-owner element counts.
 * @return Prefix-sum offset array of size `N`.
 */
template <std::size_t N>
constexpr std::array<std::size_t, N> computeOffsets(const std::array<std::size_t, N>& counts) {

    std::array<std::size_t, N> offsets{};

    std::size_t acc = 0;
    for (std::size_t i = 0; i < N; ++i) {
        offsets[i] = acc;
        acc += counts[i];
    }

    return offsets;
}

/**
 * @brief Compute the total number of elements from a count array.
 *
 * @tparam N Number of owners.
 * @param counts Per-owner element counts.
 * @return Sum of all counts.
 */
template <std::size_t N>
constexpr std::size_t computeTotalCount(const std::array<std::size_t, N>& counts) {

    std::size_t total = 0;
    for (std::size_t i = 0; i < N; ++i) {
        total += counts[i];
    }
    return total;
}

}  // namespace metkit::mars2grib::backend::master_registry::indexing

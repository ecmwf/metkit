#pragma once

/**
 * @file CompactIndexing.h
 * @brief Generic compile-time support for compact, contiguous indexing.
 *
 * This header provides generic utilities for registries whose elements
 * are organized in compact, contiguous blocks (CSR-style layout).
 *
 * Typical use cases:
 * - Concept → Variant indexing
 * - Section → Template indexing
 *
 * This file defines *pure arithmetic rules* for compact indexing.
 * It does not know anything about concepts, sections, or enums.
 *
 * @ingroup master_registry_indexing
 */

#include <cstddef>

namespace metkit::mars2grib::backend::master_registry::indexing {

/**
 * @brief Compute a global index from a base offset and a local index.
 *
 * This is the fundamental rule of all compact registries:
 *
 * \code
 * globalIndex = offset + localIndex
 * \endcode
 *
 * @param offset Base offset of the owning block.
 * @param localIndex Index inside the block.
 * @return Global index.
 */
constexpr std::size_t makeGlobalIndex(std::size_t offset, std::size_t localIndex) noexcept {
    return offset + localIndex;
}

}  // namespace metkit::mars2grib::backend::master_registry::indexing

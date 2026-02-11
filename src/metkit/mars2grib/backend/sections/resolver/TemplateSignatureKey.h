/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file TemplateSignatureKey.h
/// @brief Compact key representing an active concept-variant signature.
///
/// This header defines `TemplateSignatureKey`, an **internal, fixed-size key**
/// used by the section resolver to represent the *active concept-variant
/// signature* of an encoding request.
///
/// A signature key is constructed from the runtime active concept state and
/// encodes, in a compact and ordered form, the set of **global variant
/// identifiers** that characterize the request.
///
/// The key is used for:
/// - Efficient comparison
/// - Ordered lookup
/// - Hash-based indexing
///
/// It is designed for hot-path usage and introduces no dynamic allocation.
///
/// @note
/// This type is an internal implementation detail of the resolver and is not
/// part of the public API.
///
/// @ingroup mars2grib_backend_section_resolver
///
#pragma once

// System includes
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"

namespace metkit::mars2grib::backend::sections::resolver::detail {

///
/// @brief Fixed-size signature key for concept-variant combinations.
///
/// `TemplateSignatureKey` represents an **ordered sequence of global
/// variant identifiers** describing the active concept state.
///
/// The key is:
/// - Dense and contiguous
/// - Order-sensitive
/// - Free of dynamic allocation
///
/// The `size` field indicates how many entries in @ref data are valid.
///
/// Ordering and equality are defined lexicographically and are consistent
/// with the semantics of concept-variant matching.
///
struct TemplateSignatureKey {

    /// Registry providing global concept and variant identifiers
    using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;

    ///
    /// @brief Maximum number of variant identifiers that can be stored.
    ///
    /// This corresponds to the total number of registered variants.
    ///
    static constexpr std::size_t maxSize = GeneralRegistry::NVariants;

    ///
    /// @brief Ordered list of global variant identifiers.
    ///
    /// Only the first @ref size entries are valid.
    ///
    std::array<std::size_t, maxSize> data{};

    ///
    /// @brief Number of active entries in @ref data.
    ///
    std::uint16_t size{0};

    ///
    /// @brief Equality comparison.
    ///
    /// Two keys are equal if they have the same size and identical
    /// variant identifiers in the same order.
    ///
    bool operator==(const TemplateSignatureKey& other) const noexcept {
        if (size != other.size) {
            return false;
        }
        for (std::size_t i = 0; i < size; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }
        return true;
    }

    ///
    /// @brief Strict weak ordering.
    ///
    /// Lexicographical comparison on the stored variant identifiers,
    /// with shorter keys ordered before longer ones when prefixes match.
    ///
    bool operator<(const TemplateSignatureKey& other) const noexcept {
        const std::size_t n = std::min(size, other.size);
        for (std::size_t i = 0; i < n; ++i) {
            if (data[i] < other.data[i]) {
                return true;
            }
            if (data[i] > other.data[i]) {
                return false;
            }
        }
        return size < other.size;
    }
};

///
/// @brief Hash functor for `TemplateSignatureKey`.
///
/// This hash combines the variant identifiers using a standard
/// hash-mixing scheme suitable for unordered containers.
///
struct TemplateSignatureKeyHash {

    std::size_t operator()(const TemplateSignatureKey& key) const noexcept {

        std::size_t h = 0;
        for (std::size_t i = 0; i < key.size; ++i) {
            h ^= key.data[i] + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

///
/// @namespace metkit::mars2grib::backend::sections::resolver::detail::debug
///
/// @brief Debug and introspection utilities for template signature keys.
///
/// These utilities are intended exclusively for diagnostics and debugging.
/// They must not be used in performance-critical code paths.
///
namespace debug {

///
/// @brief Print a human-readable representation of a template signature key.
///
/// @param[in]  key    Signature key
/// @param[in]  prefix Line prefix used for indentation
/// @param[out] os     Output stream
///
inline void debug_print_Key(const TemplateSignatureKey& key, std::string_view prefix, std::ostream& os) {

    using GeneralRegistry = TemplateSignatureKey::GeneralRegistry;

    TemplateSignatureKeyHash hasher;

    os << prefix << " :: TemplateSignatureKey\n";
    os << prefix << " ::   maxSize : " << key.maxSize << "\n";
    os << prefix << " ::   size    : " << key.size << "\n";
    os << prefix << " ::   hash    : " << hasher(key) << "\n";
    os << prefix << " ::   variantIndices : [ ";

    for (std::size_t i = 0; i < key.size; ++i) {
        os << key.data[i];
        if (i + 1 < key.size) {
            os << ", ";
        }
    }

    os << " ]\n";

    os << prefix << " ::   variantNames   : [ ";

    for (std::size_t i = 0; i < key.size; ++i) {
        const std::size_t id = key.data[i];
        os << "\"" << GeneralRegistry::conceptNameArr[id] << "::" << GeneralRegistry::variantNameArr[id] << "\"";
        if (i + 1 < key.size) {
            os << ", ";
        }
    }

    os << " ]\n";
}

///
/// @brief Convert a template signature key to a JSON-like string.
///
/// Intended exclusively for debugging and diagnostics.
///
/// @param[in] key Signature key
///
/// @return JSON-style string representation
///
inline std::string debug_convert_Key_to_json(const TemplateSignatureKey& key) {

    using GeneralRegistry = TemplateSignatureKey::GeneralRegistry;

    TemplateSignatureKeyHash hasher;
    std::ostringstream oss;

    oss << "{ \"TemplateSignatureKey\": { "
        << "\"maxSize\": " << key.maxSize << ", "
        << "\"size\": " << key.size << ", "
        << "\"hash\": " << hasher(key) << ", "
        << "\"variantIndices\": [ ";

    for (std::size_t i = 0; i < key.size; ++i) {
        oss << key.data[i];
        if (i + 1 < key.size) {
            oss << ", ";
        }
    }

    oss << " ], \"variantNames\": [ ";

    for (std::size_t i = 0; i < key.size; ++i) {
        const std::size_t id = key.data[i];
        oss << "\"" << GeneralRegistry::conceptNameArr[id] << "::" << GeneralRegistry::variantNameArr[id] << "\"";
        if (i + 1 < key.size) {
            oss << ", ";
        }
    }

    oss << " ] } }";

    return oss.str();
}

}  // namespace debug

}  // namespace metkit::mars2grib::backend::sections::resolver::detail

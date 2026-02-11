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
/// @file ActiveConceptsData.h
/// @brief Runtime representation of active concept variants inferred from a MARS dictionary.
///
/// This header defines `ActiveConceptsData`, a **runtime data structure**
/// representing the set of concepts that are *semantically active* for a
/// given encoding request.
///
/// An instance of this structure is **directly inferred from a MARS input
/// dictionary** during the normalization and sanitization phases of the
/// encoding pipeline.
///
/// It captures, in a compact and lookup-friendly form:
/// - Which concepts are required to semantically describe the MARS request
/// - Which variant of each required concept must be used
///
/// The structure is designed to be consumed by the section resolver
/// subsystem as input state for template resolution.
///
/// @ingroup mars2grib_backend_section_resolver
///
#pragma once

// System includeds
#include <array>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"

namespace metkit::mars2grib::backend::sections::resolver {

///
/// @brief Runtime container describing active concepts and their variants.
///
/// `ActiveConceptsData` is a **pure data carrier** that represents the
/// semantic interpretation of a MARS dictionary in terms of concepts and
/// concept variants.
///
/// The structure answers two fundamental questions for each registered concept:
///
/// 1. **Is this concept required** to semantically describe the MARS request?
/// 2. **If required, which variant** of the concept must be used?
///
/// This information is produced by analyzing the MARS dictionary and is
/// consumed by the section resolver to select appropriate section templates.
///
/// ------------------------------------------------------------------------
///
/// @section activeconcepts_representation Internal representation
///
/// The data is stored using two complementary arrays:
///
/// - `activeVariantIndices`
/// A dense array indexed by **concept identifier**
///
/// - `activeConceptsIndices`
/// A sparse list containing only the identifiers of **active concepts**
///
/// The `count` field specifies the number of active concepts.
///
/// The two arrays are used together as follows:
///
/// @code
/// for (std::size_t i = 0; i < count; ++i) {
/// std::size_t conceptId       = activeConceptsIndices[i];
/// std::size_t globalVariantId = activeVariantIndices[conceptId];
/// }
/// @endcode
///
/// ------------------------------------------------------------------------
///
/// @section activeconcepts_semantics Semantics of activeVariantIndices
///
/// Each entry in `activeVariantIndices` encodes **both presence and choice**:
///
/// - If a concept is **not required** to describe the MARS dictionary,
/// its corresponding entry is set to a special sentinel value
/// exposed by the `GeneralRegistry` (typically referred to as `Missing`)
///
/// - If a concept **is required**, the entry contains the **global variant
/// identifier** corresponding to the variant that must be used
///
/// This design allows:
/// - O(1) access by concept identifier
/// - Explicit representation of inactive concepts
/// - Efficient iteration over only active concepts
///
/// ------------------------------------------------------------------------
///
/// @section activeconcepts_design Design considerations
///
/// - No dynamic allocation
/// - Fixed-capacity storage
/// - Trivially copyable
/// - Suitable for hot-path usage
///
/// The structure performs **no validation** and enforces **no policy**.
/// It is assumed to be fully consistent when handed to the resolver.
///
struct ActiveConceptsData {

    /// Registry providing global concept and variant identifiers
    using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;

    ///
    /// @brief Maximum number of concepts that can be represented.
    ///
    static constexpr std::size_t maxCapacity = GeneralRegistry::NConcepts;

    ///
    /// @brief Mapping from concept identifier to active variant identifier.
    ///
    /// Indexed by **concept identifier**.
    ///
    /// Semantics:
    /// - `Missing` value → concept not required
    /// - otherwise       → global variant identifier to be used
    ///
    std::array<std::size_t, maxCapacity> activeVariantIndices{};

    ///
    /// @brief Sparse list of active concept identifiers.
    ///
    /// Only the first @ref count entries are valid.
    ///
    std::array<std::size_t, maxCapacity> activeConceptsIndices{};

    ///
    /// @brief Number of active concepts.
    ///
    std::size_t count{0};
};


///
/// @namespace metkit::mars2grib::backend::sections::resolver::debug
///
/// @brief Internal utilities for diagnostics and introspection.
///
/// This namespace contains debug-only helpers used to inspect
/// `ActiveConceptsData` instances. These utilities are not part of the
/// public resolver API and must not be used in performance-critical paths.
///
namespace debug {

///
/// @brief Print a human-readable representation of active concept data.
///
/// The output explicitly reflects the canonical access pattern used by
/// the resolver and highlights both:
/// - which concepts are active
/// - which variants are selected
///
/// @param[in]  data   Active concept state
/// @param[in]  prefix Line prefix used for indentation
/// @param[out] os     Output stream
///
inline void debug_print_ActiveConceptsData(const ActiveConceptsData& data, std::string_view prefix, std::ostream& os) {

    using GeneralRegistry = ActiveConceptsData::GeneralRegistry;

    os << prefix << " :: ActiveConceptsData\n";
    os << prefix << " ::   count : " << data.count << "\n";

    for (std::size_t i = 0; i < data.count; ++i) {
        const std::size_t conceptId = data.activeConceptsIndices[i];
        const std::size_t variantId = data.activeVariantIndices[conceptId];

        os << prefix << " ::   concept[" << i << "] : " << GeneralRegistry::conceptNameArr[variantId];

        if (variantId == GeneralRegistry::missing) {
            os << " -> Missing\n";
        }
        else {
            os << " -> " << GeneralRegistry::variantNameArr[variantId] << "\n";
        }
    }
}

///
/// @brief Convert active concept data to a JSON-like string.
///
/// The resulting string is intended exclusively for debugging and
/// diagnostics. It is not guaranteed to conform to strict JSON.
///
/// @param[in] data Active concept state
///
/// @return JSON-style string representation
///
inline std::string debug_convert_ActiveConceptsData_to_json(const ActiveConceptsData& data) {

    using GeneralRegistry = ActiveConceptsData::GeneralRegistry;

    std::ostringstream oss;

    oss << "{ \"ActiveConceptsData\": { "
        << "\"count\": " << data.count << ", "
        << "\"concepts\": [ ";

    for (std::size_t i = 0; i < data.count; ++i) {
        const std::size_t conceptId = data.activeConceptsIndices[i];
        const std::size_t variantId = data.activeVariantIndices[conceptId];

        oss << "{ \"concept\": \"" << GeneralRegistry::conceptNameArr[conceptId] << "\"";

        if (variantId == GeneralRegistry::missing) {
            oss << ", \"variant\": \"Missing\" }";
        }
        else {
            oss << ", \"variant\": \"" << GeneralRegistry::variantNameArr[variantId] << "\" }";
        }

        if (i + 1 < data.count) {
            oss << ", ";
        }
    }

    oss << " ] } }";

    return oss.str();
}

}  // namespace debug

}  // namespace metkit::mars2grib::backend::sections::resolver

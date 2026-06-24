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
/// @file SectionLayoutData.h
/// @brief Runtime description of a resolved GRIB section layout.
///
/// This header defines `SectionLayoutData`, the **final product** of the
/// section resolver subsystem.
///
/// A `SectionLayoutData` instance represents a fully resolved and
/// deterministic description of how a GRIB section must be encoded.
///
/// It is produced by:
/// - Resolving declarative recipe definitions
/// - Matching them against the active concept state
/// - Selecting the appropriate template number
///
/// The structure is subsequently consumed by the header encoder to
/// drive the execution of concept operations for the section.
///
/// @ingroup mars2grib_backend_section_resolver
///
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ResolvedTemplateData.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::sections::resolver {


///
/// @brief Final resolved layout for a GRIB section.
///
/// `SectionLayoutData` is a **runtime data container** that fully describes
/// the encoding layout of a GRIB section.
///
/// It captures:
/// - The GRIB section number
/// - The selected GRIB template number
/// - The ordered list of global concept-variant identifiers that must be
/// applied when encoding the section
///
/// The structure is:
/// - Immutable once constructed
/// - Flat and cache-friendly
/// - Free of ownership and dynamic allocation
///
/// Instances of this type represent the **terminal output** of the section
/// resolution process.
///
struct SectionLayoutData {

    /// Registry providing global concept and variant identifiers
    using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;

    ///
    /// @brief Maximum number of concept variants that can be stored.
    ///
    /// This corresponds to the total number of registered concepts.
    ///
    static constexpr std::size_t maxCapacity = GeneralRegistry::NConcepts;

    ///
    /// @brief Ordered list of global variant identifiers defining the layout.
    ///
    /// Only the first @ref count entries are valid.
    ///
    std::array<std::size_t, maxCapacity> variantIndices{};

    ///
    /// @brief Number of active variants in @ref variantIndices.
    ///
    std::size_t count{0};

    ///
    /// @brief Selected GRIB template number for the section.
    ///
    std::size_t templateNumber{0};

    ///
    /// @brief GRIB section number this layout applies to.
    ///
    std::size_t sectionNumber{0};
};

///
/// @namespace metkit::mars2grib::backend::sections::resolver::detail
///
/// @brief Internal helpers for constructing section layout data.
///
/// This namespace contains non-public utilities used during section
/// resolution. Functions here are not part of the public API and may
/// change without notice.
///
namespace detail {

///
/// @brief Construct a `SectionLayoutData` from a resolved recipe entry.
///
/// This function converts a `ResolvedTemplateData` payload into a
/// `SectionLayoutData` instance by:
/// - Copying the ordered list of variant identifiers
/// - Assigning the selected template number
/// - Binding the layout to a specific GRIB section
///
/// @param[in] sectionNumber GRIB section number
/// @param[in] recipeEntry  Resolved recipe entry
///
/// @return Fully populated section layout data
///
/// @throws Mars2GribGenericException
/// If construction fails for any reason
///
inline SectionLayoutData make_SectionLayoutData_or_throw(std::size_t sectionNumber,
                                                         const dsl::ResolvedTemplateData& recipeEntry) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        SectionLayoutData layoutData{};
        layoutData.count = recipeEntry.count;

        for (std::size_t i = 0; i < recipeEntry.count; ++i) {
            layoutData.variantIndices[i] = recipeEntry.variantIndices[i];
        }

        layoutData.templateNumber = recipeEntry.templateNumber;
        layoutData.sectionNumber  = sectionNumber;

        return layoutData;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Unable to create SectionLayoutData", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace detail

///
/// @namespace metkit::mars2grib::backend::sections::resolver::debug
///
/// @brief Debug and introspection utilities for section layout data.
///
/// This namespace contains diagnostic helpers for inspecting
/// `SectionLayoutData` instances. These utilities are not intended for
/// performance-critical use.
///
namespace debug {

///
/// @brief Print a human-readable description of a section layout.
///
/// @param[in]  data   Section layout data
/// @param[in]  prefix Line prefix used for indentation
/// @param[out] os     Output stream
///
inline void debug_print_SectionLayoutData(const SectionLayoutData& data, std::string_view prefix, std::ostream& os) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    os << prefix << " :: SectionLayoutData" << std::endl;
    os << prefix << " ::   sectionNumber  : " << data.sectionNumber << std::endl;
    os << prefix << " ::   templateNumber : " << data.templateNumber << std::endl;
    os << prefix << " ::   count          : " << data.count << std::endl;
    os << prefix << " ::   variantIndices : [ ";

    for (std::size_t i = 0; i < data.count; ++i) {
        os << data.variantIndices[i];
        if (i + 1 < data.count) {
            os << ", ";
        }
    }

    os << " ] " << std::endl;

    os << prefix << " ::   variantNames : [ ";

    for (std::size_t i = 0; i < data.count; ++i) {
        std::size_t id    = data.variantIndices[i];
        std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
        std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
        os << "\"" << cname << "::" << vname << "\"";
        if (i + 1 < data.count) {
            os << ", ";
        }
    }

    os << " ]\n";
}

///
/// @brief Convert section layout data to a JSON-like string.
///
/// Intended exclusively for debugging and diagnostics.
///
/// @param[in] data Section layout data
///
/// @return JSON-style string representation
///
inline std::string debug_convert_SectionLayoutData_to_json(const SectionLayoutData& data) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    std::ostringstream oss;

    oss << "{ \"SectionLayoutData\": { "
        << "\"sectionNumber\": " << data.sectionNumber << ", "
        << "\"templateNumber\": " << data.templateNumber << ", "
        << "\"count\": " << data.count << ", "
        << "\"variantIndices\": [ ";

    for (std::size_t i = 0; i < data.count; ++i) {
        oss << data.variantIndices[i];
        if (i + 1 < data.count) {
            oss << ", ";
        }
    }

    oss << " ], \"variantNames\": [ ";

    for (std::size_t i = 0; i < data.count; ++i) {
        const std::size_t id = data.variantIndices[i];
        oss << "\"" << GeneralRegistry::conceptNameArr[id] << "::" << GeneralRegistry::variantNameArr[id] << "\"";
        if (i + 1 < data.count) {
            oss << ", ";
        }
    }

    oss << " ] } }";

    return oss.str();
}

}  // namespace debug

}  // namespace metkit::mars2grib::backend::sections::resolver

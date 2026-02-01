/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file GribHeaderLayoutData.h
 * @brief Structural blueprint of a resolved GRIB message.
 *
 * This header defines the data structures used to represent the resolved
 * structural state of a GRIB message. It acts as a bridge between the
 * frontend (Resolution) and the backend (Encoding).
 */
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionLayoutData.h"

namespace metkit::mars2grib::frontend {

/**
 * @brief Aggregated layout metadata for a complete GRIB message.
 *
 * `GribHeaderLayoutData` serves as a **blueprint** or **manifest**. It contains
 * the resolved templates and specific variants for every GRIB section (0-7).
 *
 * This structure is strictly "data-only" and is intended to be passed to
 * specialized encoders which use these indices to perform lookups in the
 * `GeneralRegistry`.
 */
struct GribHeaderLayoutData {
    using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using SectionLayoutData = metkit::mars2grib::backend::sections::resolver::SectionLayoutData;

    /** @brief Number of sections defined by the GRIB registry (typically 8)
     * we use 5 because section from 6 to 8 are pure data
     */
    static constexpr std::size_t nSections = GeneralRegistry::NSections;

    /** @brief Array of layout definitions, indexed by GRIB section number. */
    std::array<SectionLayoutData, nSections> sectionLayouts{};
};


/**
 * @namespace metkit::mars2grib::frontend::debug
 * @brief Diagnostic and serialization utilities for header layout data.
 */
namespace debug {

/**
 * @brief Serialize the header layout to a JSON-like diagnostic string.
 *
 * Produces a machine-readable representation of the resolved layout. This is
 * primarily used for regression testing (dumping "GRIB blueprints") to
 * ensure that changes in metadata resolution do not unexpectedly alter
 * the resulting GRIB structure.
 *
 * @param data The layout data to serialize.
 * @return A JSON formatted string containing the section-variant map.
 */
inline std::string debug_convert_GribHeaderLayoutData_to_json(const GribHeaderLayoutData& data) {

    using metkit::mars2grib::backend::sections::resolver::debug::debug_convert_SectionLayoutData_to_json;

    std::ostringstream oss;
    oss << "{ \"GribHeaderLayoutData\": { \"sections\": [ ";

    for (std::size_t sid = 0; sid < GribHeaderLayoutData::nSections; ++sid) {
        // Delegate to the resolution::debug helper to get the "Concept::Variant" names
        oss << debug_convert_SectionLayoutData_to_json(data.sectionLayouts[sid]);

        if (sid + 1 < GribHeaderLayoutData::nSections) {
            oss << ", ";
        }
    }

    oss << " ] } }";
    return oss.str();
}

/**
 * @brief Detailed print to ostream for human-readable logging.
 *
 * Formats the layout into a hierarchical tree view, showing which GRIB
 * Template is used for each section and listing the specific Concept/Variant
 * pairs that will be encoded.
 *
 * @param data   The layout data to print.
 * @param prefix Leading string for each line (used for indentation/log headers).
 * @param os     The output stream.
 */
inline void debug_print_GribHeaderLayoutData(const GribHeaderLayoutData& data, std::string_view prefix, std::ostream& os) {
    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    os << prefix << " :: GribHeaderLayoutData Summary\n";
    for (std::size_t sid = 0; sid < GribHeaderLayoutData::nSections; ++sid) {
        const auto& section = data.sectionLayouts[sid];
        os << prefix << " ::   Section[" << sid << "] Template: " << section.templateNumber << "\n";

        for (std::size_t i = 0; i < section.count; ++i) {
            std::size_t id = section.variantIndices[i];
            os << prefix << " ::     - " << GeneralRegistry::conceptNameArr[id]
               << "::" << GeneralRegistry::variantNameArr[id] << "\n";
        }
    }
}

}  // namespace debug
} // namespace metkit::mars2grib::frontend

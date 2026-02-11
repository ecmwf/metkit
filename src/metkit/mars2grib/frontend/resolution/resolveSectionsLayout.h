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
/// @file GribHeaderLayoutResolver.h
/// @brief Logic for mapping active semantic concepts to structural GRIB sections.
///
/// This header defines the structural resolution logic that transforms
/// normalized MARS request data (ActiveConceptsData) into a concrete
/// GRIB message blueprint (GribHeaderLayoutData).
///
/// @ingroup mars2grib_frontend_resolution
///

#pragma once

// System includes
#include <array>
#include <ostream>
#include <string_view>

// Project includes
#include "metkit/mars2grib/backend/sections/resolver/ActiveConceptsData.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionLayoutData.h"
#include "metkit/mars2grib/frontend/GribHeaderLayoutData.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/SectionTemplateSelectors.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend::resolution {

using SectionLayoutData  = metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
using ActiveConceptsData = metkit::mars2grib::backend::sections::resolver::ActiveConceptsData;

///
/// @brief Resolves the structural layout of all GRIB sections.
///
/// This function orchestrates the "Recipe Selection" phase of the frontend.
/// It iterates through every canonical GRIB section and utilizes static
/// selectors to determine:
/// 1. Which GRIB template should represent the section.
/// 2. Which concept variants should be mapped to that section's fields.
///
/// ------------------------------------------------------------------------
///
/// @section resolution_logic Resolution Logic
///
/// The resolution is deterministic and based on the @ref ActiveConceptsData.
/// For each section $S \in [0, N_{sections})$, the function:
/// - Invokes the corresponding @ref SectionTemplateSelector.
/// - Validates that the selector returned a valid payload for the correct section.
///
/// ------------------------------------------------------------------------
///
/// @param[in] activeConcepts The semantic interpretation of the MARS request.
/// @return A dense array of resolved @ref SectionLayoutData.
/// @throws Mars2GribGenericException if a section fails to resolve or returns invalid data.
///
inline std::array<SectionLayoutData, backend::concepts_::GeneralRegistry::NSections> resolve_SectionsLayout_or_throw(
    const ActiveConceptsData& activeConcepts) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using metkit::mars2grib::frontend::resolution::recipes::SectionTemplateSelectors;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Recover the static structural recipes (Stage 1 resolution)
        const auto& selectors = SectionTemplateSelectors::value;

        // Pre-allocate the layout container on the stack
        std::array<SectionLayoutData, GeneralRegistry::NSections> sectionsLayout;

        // Iterate through all canonical GRIB sections (0 through 8)
        for (std::size_t section = 0; section < GeneralRegistry::NSections; ++section) {

            // Apply the recipe for this specific section index
            SectionLayoutData sectionData = selectors[section].select_or_throw(activeConcepts);

            // Validation: Ensure the recipe actually targetted the expected section index
            if (sectionData.sectionNumber != section) {
                throw Mars2GribGenericException("SectionTemplateSelector mismatch: expected section " +
                                                    std::to_string(section) + " but recipe returned " +
                                                    std::to_string(sectionData.sectionNumber),
                                                Here());
            }

            sectionsLayout[section] = std::move(sectionData);
        }

        return sectionsLayout;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Critical failure: Unable to resolve GRIB HeaderLayout", Here()));
    }

    mars2gribUnreachable();
}

///
/// @namespace metkit::mars2grib::frontend::resolution::debug
/// @brief Diagnostic tools for the resolution process.
///
namespace debug {

///
/// @brief Logs the resolution results for debugging purposes.
/// * @param[in] layout Resolved section layouts
/// @param[out] os    Target output stream
///
inline void debug_print_resolved_layout(
    const std::array<SectionLayoutData, metkit::mars2grib::backend::concepts_::GeneralRegistry::NSections>& layout,
    std::ostream& os) {

    os << "--- GRIB Layout Resolution Debug ---" << std::endl;
    for (const auto& s : layout) {
        os << "Section " << s.sectionNumber << " -> Template " << s.templateNumber << " (Concepts: " << s.count << ")"
           << std::endl;
    }
}

}  // namespace debug

}  // namespace metkit::mars2grib::frontend::resolution
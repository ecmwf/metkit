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
/// @file make_HeaderLayout.h
/// @brief Top-level factory for GRIB header structural resolution.
///
/// This header defines the entry point for the resolution subsystem. It
/// coordinates the semantic inference of concepts and the subsequent
/// mapping of those concepts to a physical GRIB section layout.
///
/// @ingroup mars2grib_frontend_resolution
///

#pragma once

// System includes
#include <array>
#include <utility>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ActiveConceptsData.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionLayoutData.h"
#include "metkit/mars2grib/frontend/GribHeaderLayoutData.h"
#include "metkit/mars2grib/frontend/resolution/resolveActiveConcepts.h"
#include "metkit/mars2grib/frontend/resolution/resolveSectionsLayout.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend {


///
/// @brief Orchestrates the complete resolution of a GRIB message blueprint.
///
/// This factory function executes the two-stage resolution pipeline:
/// * 1. **Semantic Resolution**: Infers which concepts and variants are
/// active based on the input MARS and Options dictionaries.
/// 2. **Structural Resolution**: Maps those active concepts to specific
/// GRIB sections and selects the appropriate GRIB templates.
///
/// The resulting @ref GribHeaderLayoutData is a POD-like structure suitable
/// for move-construction into the @ref SpecializedEncoder.
///
/// ------------------------------------------------------------------------
/// * @section resolution_pipeline Data Flow
///
/// 1. `MarsDict_t` / `OptDict_t` -> @ref resolve_ActiveConcepts_or_throw
/// 2. `ActiveConceptsData`      -> @ref resolve_SectionsLayout_or_throw
/// 3. `SectionLayoutData[]`     -> @ref GribHeaderLayoutData
///
/// ------------------------------------------------------------------------
///
/// @tparam MarsDict_t Type of the MARS dictionary.
/// @tparam OptDict_t  Type of the encoding options dictionary.
/// * @param[in] marsDict Input MARS request.
/// @param[in] optDict  Encoder configuration and options.
/// * @return A fully resolved @ref GribHeaderLayoutData.
/// @throws Mars2GribGenericException if any phase of the resolution fails.
///
template <class MarsDict_t, class OptDict_t>
GribHeaderLayoutData make_HeaderLayout_or_throw(const MarsDict_t& marsDict, const OptDict_t& optDict) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using metkit::mars2grib::backend::sections::resolver::ActiveConceptsData;
    using metkit::mars2grib::frontend::GribHeaderLayoutData;
    using metkit::mars2grib::frontend::resolution::resolve_ActiveConcepts_or_throw;
    using metkit::mars2grib::frontend::resolution::resolve_SectionsLayout_or_throw;
    using metkit::mars2grib::frontend::resolution::SectionLayoutData;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Step 1: Semantic Analysis (What concepts are we encoding?)
        ActiveConceptsData activeConcepts = resolve_ActiveConcepts_or_throw(marsDict, optDict);

        // Step 2: Structural Mapping (Where do these concepts live in the GRIB sections?)
        std::array<SectionLayoutData, GeneralRegistry::NSections> sectionsLayout =
            resolve_SectionsLayout_or_throw(activeConcepts);

        // Step 3: Blueprint Aggregation
        // We move the resolved array into the layout carrier to ensure zero-copy transfer.
        return GribHeaderLayoutData{{std::move(sectionsLayout)}};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Critical failure: Unable to resolve GRIB HeaderLayout", Here()));
    }

    return {};
}


namespace tests {

///
/// @brief Generates a JSON diagnostic capture of the resolution pipeline.
/// * This creates an object containing:
/// 1. The original MARS request.
/// 2. The resolved ActiveConcepts (Semantic layer).
/// 3. The resolved GribHeaderLayout (Structural layer).
/// * @return std::string A single JSON object string.
///
template <class MarsDict_t, class OptDict_t>
std::string capture_resolution_state_json(const MarsDict_t& mars, const OptDict_t& opt) {

    using metkit::mars2grib::backend::sections::resolver::debug::debug_convert_ActiveConceptsData_to_json;
    using metkit::mars2grib::frontend::debug::debug_convert_GribHeaderLayoutData_to_json;
    using metkit::mars2grib::utils::dict_traits::dict_to_json;

    // 1. Resolve states
    // Note: In a real test, you'd likely catch exceptions here to log failures
    auto activeConcepts = resolution::resolve_ActiveConcepts_or_throw(mars, opt);
    auto headerLayout   = make_HeaderLayout_or_throw(mars, opt);

    // 2. Build the aggregate JSON string
    // We leverage the debug helpers we built in previous steps
    std::ostringstream oss;
    oss << "{ "
        << "\"mars\": " << dict_to_json<MarsDict_t>(mars) << ", "
        << "\"activeConcepts\": " << debug_convert_ActiveConceptsData_to_json(activeConcepts) << ", "
        << "\"headerLayout\": " << debug_convert_GribHeaderLayoutData_to_json(headerLayout) << " }";

    return oss.str();
}

}  // namespace tests


}  // namespace metkit::mars2grib::frontend
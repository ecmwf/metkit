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
/// @file resolve_ActiveConcepts.h
/// @brief Logic for mapping MARS metadata to active GRIB concept variants.
///
/// This header defines the core resolution engine that identifies which
/// GRIB entities (Concepts and Variants) are triggered by a specific
/// combination of MARS and auxiliary metadata.
///
#pragma once

// project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/concepts/MatchingCallbacksRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ActiveConceptsData.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend::resolution {

using ActiveConceptsData = metkit::mars2grib::backend::sections::resolver::ActiveConceptsData;

///
/// @brief Orchestrates the resolution of MARS metadata into GRIB concepts.
///
/// This function is the primary entry point for the **Resolution Phase**. It iterates
/// through the global registry of GRIB concepts and executes specialized "matching
/// callbacks" against the input dictionaries.
///
/// Each callback determines which specific variant of a GRIB concept (e.g., which
/// type of Level or Step) is triggered by the current MARS request.
///
/// ### Error Handling
/// - Throws `Mars2GribGenericException` if the registry is inconsistent.
/// - Uses `std::throw_with_nested` to preserve the trace of matching failures
/// within specific callbacks.
///
/// @tparam MarsDict_t Type of the MARS metadata dictionary.
/// @tparam OptDict_t  Type of the options/configuration dictionary.
///
/// @param[in] marsDict The sanitized MARS request.
/// @param[in] optDict  The auxiliary metadata and encoding options.
///
/// @return An `ActiveConceptsData` object containing the indices of all triggered variants.
///
template <class MarsDict_t, class OptDict_t>
ActiveConceptsData resolve_ActiveConcepts_or_throw(const MarsDict_t& marsDict, const OptDict_t& optDict) {

    // bring in GeneralRegistry and MatchersRegistry
    using metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;
    using Registry = metkit::mars2grib::backend::concepts_::MatchingCallbacksRegistry<MarsDict_t, OptDict_t>;

    try {

        // Lookup of the static registry
        const auto& callbacks = Registry::matchingCallbacks;

        // Error handling
        if (callbacks.size() != GeneralRegistry::NConcepts) {
            throw Mars2GribGenericException("Wrong size of Matchers", Here());
        }

        ///
        /// @note The Matchers are typically ordered by Section (0 to 5) to ensure
        /// dependency resolution flows correctly.
        ///
        ActiveConceptsData activeConceptsData{};
        activeConceptsData.count = 0;
        for (std::size_t i = 0; i < GeneralRegistry::NConcepts; ++i) {
            std::size_t localVariantId  = callbacks[i](marsDict, optDict);
            std::size_t globalVariantId = GeneralRegistry::conceptOffsets[i] + localVariantId;
            if (localVariantId != GeneralRegistry::not_applicable) {
                activeConceptsData.activeVariantIndices[i]                           = globalVariantId;
                activeConceptsData.activeConceptsIndices[activeConceptsData.count++] = i;
            }
            else {
                activeConceptsData.activeVariantIndices[i] = GeneralRegistry::not_applicable;
            }
        }

        // Return the active concepts
        return activeConceptsData;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Unable to match ActiveConcepts", Here()));
    }

    mars2gribUnreachable();
};


}  // namespace metkit::mars2grib::frontend::resolution
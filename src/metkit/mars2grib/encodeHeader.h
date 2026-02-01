/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

// Project includes
#include "metkit/mars2grib/frontend/normalization/normalization.h"
#include "metkit/mars2grib/frontend/resolution/make_HeaderLayout.h"
#include "metkit/mars2grib/backend/SpecializedEncoder.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib {

/**
 * @brief Main entry point for the fully templated MARS-to-GRIB encoding pipeline.
 * * This orchestrator performs dual-normalization, structural resolution, and
 * plan-based execution in a single atomic flow.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
std::unique_ptr<OutDict_t> encodeHeader(const MarsDict_t& mars,
                                        const ParDict_t& par,
                                        const OptDict_t& opt,
                                        const eckit::Value& language) {

    using namespace metkit::mars2grib::frontend;
    using namespace metkit::mars2grib::backend;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // --- PHASE 1: Normalization (MARS & Parameter) ---

        // Sanitize MARS request
        MarsDict_t marsScratch;
        const MarsDict_t& activeMars = normalization::sanitize_MiscDict_if_enabled(mars, opt, language, marsScratch);

        // Sanitize Parameter metadata
        ParDict_t parScratch;
        const ParDict_t& activePar = normalization::sanitize_MiscDict_if_enabled(par, opt, language, parScratch);

        // --- PHASE 2: Structural Resolution ---

        // Resolve the message blueprint using the sanitized MARS data
        auto headerLayout = make_HeaderLayout_or_throw(activeMars, opt);

        // --- PHASE 3: Specialized Execution ---

        // Inline construction builds the optimized EncodingPlan.
        // The encode step then executes the plan using the sanitized dictionary references.
        return SpecializedEncoder<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(std::move(headerLayout))
               .encode(activeMars, activePar, opt);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Top-level encoding pipeline failure", Here()));
    }
}

} // namespace metkit::mars2grib
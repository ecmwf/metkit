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
 * @file localTablesVersion.h
 * @brief Deduction of the GRIB Local Tables Version Number.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **GRIB Local Tables Version Number**
 * (`localTablesVersionNumber`).
 *
 * At present, the deduction returns a fixed value indicating that
 * **no local GRIB tables are in use** and that only standard GRIB
 * tables apply.
 *
 * This deduction exists to provide a single, explicit control point
 * for future support of local GRIB tables.
 *
 * Deductions are responsible for:
 * - providing deterministic values for encoding logic
 * - centralizing policy decisions related to GRIB metadata
 *
 * Deductions:
 * - do NOT encode GRIB keys directly
 * - do NOT apply inference or data-driven logic unless explicitly required
 *
 * Error handling:
 * - this deduction is total and cannot fail
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value derived via deduction logic (including fixed defaults)
 *
 * @section References
 * Concept:
 *   - @ref identificationEncoding.h
 *
 * Related deductions:
 *   - @ref centre.h
 *   - @ref subCentre.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB Local Tables Version Number.
 *
 * @section Deduction contract
 * - Reads: none
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: none
 *
 * This deduction resolves the GRIB
 * `localTablesVersionNumber`.
 *
 * The value is currently **fixed to `0`**, indicating that
 * no local GRIB tables are in use and that only standard
 * GRIB tables apply.
 *
 * This function serves as the single authoritative location
 * for future logic related to local GRIB tables.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The local tables version number.
 *   Currently always returns `0`.
 *
 * @note
 *   A return value of `0` explicitly signals that
 *   no local GRIB tables are active.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_LocalTablesVersion_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    // Defaulting local Tables version to 0
    long localTablesVersion = 0L;

    // Emit RESOLVE log entry
    MARS2GRIB_LOG_RESOLVE([&]() {
        std::string logMsg = "`localTablesVersionNumber` resolved from input dictionaries: value='";
        logMsg += std::to_string(localTablesVersion) + "'";
        return logMsg;
    }());

    // Success exit point
    return localTablesVersion;
};

}  // namespace metkit::mars2grib::backend::deductions

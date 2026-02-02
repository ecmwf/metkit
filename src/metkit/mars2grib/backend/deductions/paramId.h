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
 * @file paramId.h
 * @brief Deduction of the GRIB `paramId` identifier.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **GRIB parameter identifier** (`paramId`) from input
 * dictionaries.
 *
 * The deduction retrieves the parameter identifier explicitly from the
 * MARS dictionary and returns it verbatim as a numeric value.
 * No inference, defaulting, normalization, or GRIB table validation is
 * performed at this stage.
 *
 * Error handling follows a strict fail-fast strategy:
 * - missing or invalid inputs cause immediate failure
 * - errors are reported using domain-specific deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value resolved from one or more input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref paramEncoding.h
 *
 * Related deductions:
 *   - @ref level.h
 *   - @ref pvArray.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// exception and logging
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB parameter identifier (`paramId`).
 *
 * @section Deduction contract
 * - Reads: `mars["param"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws
 *
 * This deduction resolves the GRIB parameter identifier associated with
 * the field being encoded.
 *
 * The value is treated as mandatory and is returned verbatim as a
 * numeric identifier. No semantic interpretation or validation against
 * GRIB parameter tables is performed.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide the key `param`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused).
 *
 * @param[in] mars
 *   MARS dictionary from which the parameter identifier is resolved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The resolved GRIB parameter identifier.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If the key `param` is missing, cannot be converted to `long`,
 *   or if any unexpected error occurs during deduction.
 *
 * @note
 *   This deduction assumes that the parameter identifier is explicitly
 *   provided by MARS and does not attempt any inference or defaulting.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ParamId_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS param
        long paramId = get_or_throw<long>(mars, "param");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`paramId` resolved from input dictionaries: value='";
            logMsg += std::to_string(paramId) + "'";
            return logMsg;
        }());

        // Success exit point
        return paramId;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `paramId` from input dictionaries", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

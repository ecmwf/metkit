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
 * @file methodNumber.h
 * @brief Deduction of the wave processing method identifier.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **wave processing method identifier** (`methodNumber`)
 * from MARS metadata.
 *
 * The deduction retrieves the method identifier explicitly from the
 * MARS dictionary and returns it verbatim, without applying inference,
 * defaulting, or semantic interpretation.
 *
 * Deductions are responsible for:
 * - extracting values from MARS, parameter, and option dictionaries
 * - enforcing deterministic resolution rules
 * - returning strongly typed values to concept operations
 *
 * Deductions:
 * - do NOT encode GRIB keys directly
 * - do NOT apply heuristic or data-driven inference
 * - do NOT validate against GRIB code tables unless explicitly required
 *
 * Error handling follows a strict fail-fast strategy:
 * - missing or invalid inputs cause immediate failure
 * - errors are reported using domain-specific deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value resolved directly from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref longrangeEncoding.h
 *
 * Related deductions:
 *   - @ref systemNumber.h
 *
 * @ingroup mars2grib_backend_deductions
 */

#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the wave processing method identifier.
 *
 * @section Deduction contract
 * - Reads: `mars["method"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws
 *
 * This deduction retrieves the wave processing method identifier
 * from the MARS dictionary.
 *
 * The value is treated as mandatory and is returned verbatim as a
 * numeric identifier. No inference, defaulting, or validation against
 * GRIB code tables is performed.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide the key `method`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused).
 *
 * @param[in] mars
 *   MARS dictionary from which the method identifier is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The wave processing method identifier.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If the key `method` is missing, cannot be converted to `long`,
 *   or if any unexpected error occurs during deduction.
 *
 * @note
 *   This deduction assumes that the method identifier is explicitly
 *   provided by the MARS dictionary and does not attempt any semantic
 *   interpretation or consistency checking.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_MethodNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS method
        long methodNumber = get_or_throw<long>(mars, "method");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`methodNumber` resolved from input dictionaries: value=";
            logMsg += std::to_string(methodNumber);
            return logMsg;
        }());

        // Success exit point
        return methodNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `method` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

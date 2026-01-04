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
 * @file model.h
 * @brief Deduction of the MARS model identifier.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **model identifier** from MARS metadata.
 *
 * The deduction retrieves the model identifier explicitly from the
 * MARS dictionary and returns it verbatim. No inference, defaulting,
 * normalization, or validation against GRIB tables is performed.
 *
 * Deductions are responsible for:
 * - extracting values from MARS, parameter, and option dictionaries
 * - applying deterministic deduction logic
 * - returning strongly typed values to concept operations
 *
 * Deductions:
 * - do NOT encode GRIB keys directly
 * - do NOT infer defaults
 * - do NOT perform GRIB table validation
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
 *   - @ref destineEncoding.h
 *
 * Related deductions:
 *   - @ref experiment.h
 *   - @ref expver.h
 *   - @ref generation.h
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
 * @brief Resolve the model identifier from the MARS dictionary.
 *
 * @section Deduction contract
 * - Reads: `mars["model"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws
 *
 * This deduction retrieves the model identifier from the MARS dictionary.
 *
 * The value is treated as mandatory and is returned verbatim as a string.
 * No semantic interpretation, validation, or normalization is applied.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide the key `model`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused).
 *
 * @param[in] mars
 *   MARS dictionary from which the model identifier is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The model identifier as provided by the MARS dictionary.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If the key `model` is missing, cannot be converted to `std::string`,
 *   or if any unexpected error occurs during deduction.
 *
 * @note
 *   This deduction assumes that the model identifier is explicitly
 *   provided by MARS and does not attempt any inference or defaulting.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Model_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS model
        std::string marsModelVal = get_or_throw<std::string>(mars, "model");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`model` resolved from input dictionaries: value='" + marsModelVal + "'";
            return logMsg;
        }());

        // Success exit point
        return marsModelVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `model` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

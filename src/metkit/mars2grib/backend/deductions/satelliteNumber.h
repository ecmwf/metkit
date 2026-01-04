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
 * @file satelliteNumber.h
 * @brief Deduction of the satellite platform identifier from MARS metadata.
 *
 * This header defines the deduction responsible for resolving the
 * satellite identifier used in satellite-based products.
 *
 * The deduction extracts the identifier from the MARS dictionary
 * and returns it verbatim without interpretation or validation.
 *
 * Deductions:
 * - extract values from MARS, parameter, or option dictionaries
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer missing values
 * - normalize or validate semantics
 * - perform GRIB table validation
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: value obtained through deduction logic from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref satelliteEncoding.h
 *
 * Related deductions:
 *   - @ref instrumentType.h
 *   - @ref channel.h
 *   - @ref satelliteSeries.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System include
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the satellite identifier from the MARS dictionary.
 *
 * @section Deduction contract
 * - Reads: `mars["ident"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws
 *
 * This deduction retrieves the mandatory `ident` entry from the
 * MARS dictionary and returns it as a numeric satellite identifier.
 *
 * No inference, defaulting, normalization, or semantic validation
 * is performed.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide the key `ident`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused).
 *
 * @param[in] mars
 *   MARS dictionary providing the satellite identifier.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   Satellite identifier resolved from the MARS dictionary.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If the key `ident` is missing, cannot be retrieved as a `long`,
 *   or if any unexpected error occurs.
 *
 * @note
 *   This deduction is deterministic and does not depend on any
 *   pre-existing GRIB header state.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_satelliteNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory satellite identifier from MARS dictionary
        long satelliteNumber = get_or_throw<long>(mars, "ident");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`satelliteNumber` resolved from MARS dictionary: value='";
            logMsg += std::to_string(satelliteNumber);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return satelliteNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `satelliteNumber` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

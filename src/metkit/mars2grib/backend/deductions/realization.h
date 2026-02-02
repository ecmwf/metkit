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
 * @file realization.h
 * @brief Deduction of the GRIB realization identifier.
 *
 * This file defines the deduction responsible for resolving the
 * **realization identifier**, which distinguishes individual realizations
 * (members) within an ensemble, stochastic system, or DestinE workflow.
 *
 * The realization identifier is retrieved explicitly from the MARS
 * dictionary and passed verbatim to the encoder. No inference, defaulting,
 * or normalization is performed.
 *
 * Deductions in this file are responsible for:
 * - extracting realization metadata from the MARS dictionary
 * - applying deterministic, fail-fast resolution rules
 * - returning a strongly typed value to the encoding layer
 *
 * Deductions:
 * - do NOT infer realization semantics
 * - do NOT validate ensemble consistency
 * - do NOT depend on pre-existing GRIB header state
 *
 * Error handling follows a strict fail-fast policy:
 * - missing or invalid inputs cause immediate failure
 * - errors are reported using Mars2Grib deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value retrieved directly from the MARS dictionary
 *
 * @section References
 * Concept:
 *   - @ref destineEncoding.h
 *
 * Related deductions:
 *   - @ref activity.h
 *   - @ref experiment.h
 *   - @ref generation.h
 *   - @ref model.h
 *   - @ref resolution.h
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
 * @brief Resolve the realization identifier from the MARS dictionary.
 *
 * @section Deduction contract
 * - Reads: `mars["realization"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws
 *
 * This deduction retrieves the mandatory `realization` key from the
 * MARS dictionary and returns it as a numeric identifier.
 *
 * The realization identifier is used to distinguish individual
 * realizations within ensemble or DestinE products. Its numerical
 * semantics are defined by upstream MARS and encoding conventions
 * and are not interpreted here.
 *
 * No inference, defaulting, or validation of the realization value
 * is performed.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide the key `realization`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary providing the realization identifier.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The realization identifier as provided by the MARS dictionary.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `realization` is missing from the MARS dictionary
 *   - the value cannot be converted to `long`
 *   - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction is fully deterministic.
 * - The returned value is passed verbatim to downstream encoding logic.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_Realization_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS realization
        long marsRealizationVal = get_or_throw<long>(mars, "realization");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`realization` resolved from MARS dictionary: value='";
            logMsg += std::to_string(marsRealizationVal);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return marsRealizationVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `realization` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

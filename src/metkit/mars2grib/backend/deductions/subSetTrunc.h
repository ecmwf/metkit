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
 * @file subSetTrunc.h
 * @brief Deduction of the spectral subset truncation parameter.
 *
 * This header defines the deduction responsible for resolving the
 * spectral subset truncation parameter used in spectral packing
 * configurations.
 *
 * The value is obtained from the parameter dictionary when provided.
 * If absent, a deterministic default is applied.
 *
 * Deductions:
 * - extract values from input dictionaries
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer values from MARS metadata
 * - apply implicit or hidden defaults
 * - validate against spectral grid constraints
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: value obtained or defaulted from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref packingEncoding.h
 *
 * Related deductions:
 *   - @ref bitsPerValue.h
 *   - @ref laplacianOperator.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <algorithm>
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB spectral subset truncation parameter.
 *
 * This deduction resolves the spectral subset truncation parameter
 * used to define a reduced set of spectral coefficients.
 *
 * Resolution rules:
 * - If `par::subSetTruncation` is present, its value is used directly.
 * - If `par::subSetTruncation` is absent, the value defaults explicitly
 *   to `20`.
 *
 * No inference from MARS metadata is performed.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] geo  Geometry dictionary
 * @param[in] par  Parameter dictionary; may contain `subSetTruncation`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved spectral subset truncation value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If an unexpected error occurs during dictionary access
 *
 * @note
 * This deduction is fully deterministic and does not depend on
 * any pre-existing GRIB header state.
 */
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t>
long resolve_SubSetTruncation_or_throw(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par,
                                       const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // subSetTruncation must not be larger than any pentagonalResolutionParameter
        long pentagonalResolutionParameterJ = get_or_throw<long>(geo, "pentagonalResolutionParameterJ");
        long pentagonalResolutionParameterK = get_or_throw<long>(geo, "pentagonalResolutionParameterK");
        long pentagonalResolutionParameterM = get_or_throw<long>(geo, "pentagonalResolutionParameterM");
        long defaultSubSetTrunc             = std::min(
            {20L, pentagonalResolutionParameterJ, pentagonalResolutionParameterK, pentagonalResolutionParameterM});

        // Retrieve optional subSetTruncation from parameter dictionary
        long subSetTrunc = get_opt<long>(par, "subSetTruncation").value_or(defaultSubSetTrunc);

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`subSetTruncation` resolved from input dictionaries: value='";
            logMsg += std::to_string(subSetTrunc);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return subSetTrunc;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `subSetTruncation` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file subSetTrunc.h
/// @brief Deduction of the spectral subset truncation parameter.
///
/// This header defines the deduction responsible for resolving the
/// spectral subset truncation parameter used in spectral packing
/// configurations.
///
/// The value is obtained from the parameter dictionary when provided.
/// If absent, a deterministic default is applied.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - OVERRIDE: value overridden from input dictionaries
/// - DEFAULT: value defaulted due to missing input
///
/// @section References
/// Concept:
/// - @ref packingEncoding.h
///
/// Related deductions:
/// - @ref bitsPerValue.h
/// - @ref laplacianOperator.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <algorithm>
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB spectral subset truncation parameter.
///
/// This deduction resolves the spectral subset truncation parameter
/// used to define a reduced set of spectral coefficients.
///
/// Resolution rules:
/// - If `par::subSetTruncation` is present and valid, its value is used directly.
/// - If `par::subSetTruncation` is absent, the value defaults explicitly to the
///       smaller of the MARS truncation (`mars::truncation`) and a fixed maximum of 20.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary providing `truncation` for defaulting and validation
/// @param[in] par  Parameter dictionary; may contain `subSetTruncation`
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved spectral subset truncation value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// - If an unexpected error occurs during dictionary access
/// - If `par::subSetTruncation` is provided but exceeds the MARS truncation or is negative
/// - If MARS truncation is invalid when needed for defaulting
///
/// @note
/// This deduction is fully deterministic and does not depend on
/// any pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SubSetTruncation_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // subSetTruncation must not be larger than any pentagonalResolutionParameter
        // NOTE: Mars keyword truncation is equivalent to pentagonalResolutionParameter{J,K,M}
        //       At ECMWF we cannot produce spherical harmonics with different values for J/K/M
        const auto marsTruncation = get_or_throw<long>(mars, "truncation");
        if (marsTruncation < 0) {
            std::string logMsg = "Invalid MARS truncation: value='" + std::to_string(marsTruncation) + "' is negative";
            throw Mars2GribDeductionException(logMsg, Here());
        }
        long defaultSubSetTrunc = std::min(20L, marsTruncation);

        if (has(par, "subSetTruncation")) {

            // Retrieve subSetTruncation from parameter dictionary
            long subSetTrunc = get_or_throw<long>(par, "subSetTruncation");

            // Validate that subSetTruncation does not exceed MARS truncation
            if (subSetTrunc < 0) {
                std::string logMsg = "Invalid `subSetTruncation`:";
                logMsg += " value='" + std::to_string(subSetTrunc);
                logMsg += "' is negative";
                throw Mars2GribDeductionException(logMsg, Here());
            }
            if (subSetTrunc > marsTruncation) {
                std::string logMsg = "Invalid `subSetTruncation`:";
                logMsg += " value='" + std::to_string(subSetTrunc) + "'";
                logMsg += " exceeds MARS truncation='" + std::to_string(marsTruncation) + "'";
                throw Mars2GribDeductionException(logMsg, Here());
            }

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`subSetTruncation` overridden from input dictionaries: value='";
                logMsg += std::to_string(subSetTrunc);
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return subSetTrunc;
        }
        else {

            // Emit DEFAULT log entry for defaulting
            MARS2GRIB_LOG_DEFAULT([&]() {
                std::string logMsg = "`subSetTruncation` defaulted from input dictionaries: value='";
                logMsg += std::to_string(defaultSubSetTrunc);
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return defaultSubSetTrunc;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `subSetTruncation` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file scaleFactorOfCentralWaveNumber.h
/// @brief Deduction of the GRIB `scaleFactorOfCentralWaveNumber` key.
///
/// This file defines the deduction responsible for resolving the GRIB
/// `scaleFactorOfCentralWaveNumber` key used in satellite-based products.
///
/// Together with `scaledValueOfCentralWaveNumber`, this value is used
/// to encode the central wave number according to the GRIB specification.
///
/// The deduction:
/// - reads exclusively from the parameter dictionary
/// - performs no inference or defaulting
/// - emits structured diagnostic logging
///
/// Error handling follows a fail-fast strategy with nested exception
/// propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref satelliteEncoding.h
///
/// Related deductions:
/// - @ref scaledValueOfCentralWaveNumber.h
/// - @ref channel.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `scaleFactorOfCentralWaveNumber` identifier.
///
/// @section Deduction contract
/// - Reads: `par["scaleFactorOfCentralWaveNumber"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction retrieves the scale factor applied to the central wave
/// number in satellite products.
///
/// The value is retrieved verbatim from the parameter dictionary.
/// No inference from MARS metadata and no validation of the numerical
/// range is performed.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary (unused).
///
/// @tparam ParDict_t
/// Type of the parameter dictionary. Must provide
/// `scaleFactorOfCentralWaveNumber`.
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary (unused).
///
/// @param[in] par
/// Parameter dictionary containing the scale factor.
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// Scale factor of the central wave number.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `scaleFactorOfCentralWaveNumber` is missing, cannot be
/// retrieved as a `long`, or if any unexpected error occurs.
///
/// @note
/// This deduction is deterministic and independent of GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ScaleFactorOfCentralWaveNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                     const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve scale factor of central wave number from parameter dictionary
        auto scaleFactorOfCentralWaveNumberVal = get_or_throw<long>(par, "scaleFactorOfCentralWaveNumber");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`scaleFactorOfCentralWaveNumber` resolved from parameter dictionary: value='";
            logMsg += std::to_string(scaleFactorOfCentralWaveNumberVal);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return scaleFactorOfCentralWaveNumberVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `scaleFactorOfCentralWaveNumber` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

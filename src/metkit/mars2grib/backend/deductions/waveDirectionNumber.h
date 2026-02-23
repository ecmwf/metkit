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
/// @file waveDirectionNumber.h
/// @brief Deduction of the GRIB wave direction number.
///
/// This header defines the deduction responsible for resolving the
/// GRIB wave direction number used in spectral wave products.
///
/// The value is obtained directly from MARS metadata and represents
/// the wave direction index associated with the encoded field.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values
/// - apply defaults or fallbacks
/// - validate semantic correctness
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref waveEncoding.h
///
/// Related deductions:
/// - @ref periodItMin.h
/// - @ref periodItMax.h
/// - @ref waveDirectionGrid.h
/// - @ref waveFrequencyGrid.h
/// - @ref waveFrequencyNumber.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB wave direction number.
///
/// This deduction retrieves the wave direction number from the
/// MARS dictionary using the mandatory key `direction`.
///
/// The resolved value represents the wave direction index used
/// in spectral wave products and GRIB encoding.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary (unused)
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary; must contain the key `direction`
/// @param[in] par  Parameter dictionary (unused)
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved wave direction number
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - the key `direction` is missing
/// - the value cannot be converted to `long`
/// - any unexpected error occurs
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_WaveDirectionNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory wave direction number from MARS dictionary
        auto waveDirectionNumber = get_or_throw<long>(mars, "direction");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`waveDirectionNumber` resolved from input dictionaries: value='";
            logMsg += std::to_string(waveDirectionNumber);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return waveDirectionNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `waveDirectionNumber` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

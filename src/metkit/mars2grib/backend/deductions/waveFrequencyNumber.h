// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file waveFrequencyNumber.h
/// @brief Deduction of the GRIB wave frequency number.
///
/// This header defines the deduction responsible for resolving the
/// wave frequency index used in spectral wave products.
///
/// The deduction extracts the frequency index directly from the
/// MARS dictionary and exposes it for use in GRIB encoding.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values
/// - apply defaults or fallbacks
/// - validate semantic correctness of indices
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
/// - @ref waveDirectionNumber.h
/// - @ref waveFrequencyGrid.h
/// - @ref waveFrequencyGrid.h
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
/// @brief Resolve the GRIB wave frequency number.
///
/// This deduction resolves the wave frequency index required for
/// spectral wave encoding by retrieving it from the MARS dictionary.
///
/// The value is treated as mandatory and must be provided explicitly
/// via the MARS key `frequency`.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary (unused)
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary; must contain the key `frequency`
/// @param[in] par  Parameter dictionary (unused)
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved wave frequency number
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - the key `frequency` is missing from the MARS dictionary
/// - the value cannot be converted to `long`
/// - any unexpected error occurs during deduction
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_WaveFrequencyNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory wave frequency number from MARS dictionary
        auto waveFrequencyNumber = get_or_throw<long>(mars, "frequency");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`waveFrequencyNumber` resolved from input dictionaries: value='";
            logMsg += std::to_string(waveFrequencyNumber);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return waveFrequencyNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `waveFrequencyNumber` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file firstWavelength.h
/// @brief Deduction of the first wavelength in meters for composition encoding.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **first wavelength** from MARS metadata.
///
/// The deduction retrieves the wavelength from the MARS dictionary
/// (expressed in nanometers) and converts it to meters before exposing
/// the value to the encoding layer.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying explicit and minimal transformation logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply inference, defaulting, or consistency checks
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or malformed inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value derived via deduction logic from input dictionaries
/// - OVERRIDE: value provided by parameter dictionary overriding deduction logic
///
/// @section References
/// Concept:
/// - @ref compositionEncoding.h
///
/// Related deductions:
/// - @ref chemId.h
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
/// @brief Resolve the first wavelength in meters from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["wavelength"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the first wavelength by retrieving the
/// mandatory MARS key `wavelength` (expressed in nanometers) and
/// converting it to meters.
///
/// The conversion follows:
/// \f$ \text{wavelength\_m} = \text{wavelength\_nm} \times 10^{-9} \f$
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `wavelength`
/// and conversion to `long`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the wavelength is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved wavelength in meters as a `double`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `wavelength` is missing, cannot be converted to `long`,
/// or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction assumes that the MARS `wavelength` value is expressed
/// in nanometers. Alternative units are not supported.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
double resolve_FirstWavelength_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS wavelength (in nanometers)
        long wavelengthInNanometers = get_or_throw<long>(mars, "wavelength");

        // Convert nanometers to meters
        double wavelengthInMeters = static_cast<double>(wavelengthInNanometers) / 1000000000.0;

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`firstWavelength` deduced from mars dictionary: ";
            logMsg += std::to_string(wavelengthInMeters) + " [meters]";
            logMsg += " (from " + std::to_string(wavelengthInNanometers) + " [nanometers])";
            return logMsg;
        }());

        // Success exit point
        return wavelengthInMeters;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `firstWavelength` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

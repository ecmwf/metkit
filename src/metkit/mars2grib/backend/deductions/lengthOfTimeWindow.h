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
/// @file lengthOfTimeWindow.h
/// @brief Deduction of the GRIB lengthOfTimeWindow (in seconds).
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the GRIB lengthOfTimeWindow key from input dictionaries.
///
/// The deduction reads the parameter dictionary entry lengthOfTimeWindow,
/// interprets it as hours, and converts it to seconds. If the key is missing,
/// the deduction returns the GRIB missing code 0xFFFF.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying explicit, deterministic deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT infer units or values beyond the documented rule
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or malformed inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value derived from the parameter dictionary
/// - DEFAULT: value defaulted to the GRIB missing code
///
/// @section References
/// Concept:
/// - @ref analysisEncoding.h
///
/// Related deductions:
/// - @ref offsetToEndOf4DvarWindow.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System Include
#include <exception>

// Other project includes
#include "eckit/log/Log.h"

// Core deduction includes
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `lengthOfTimeWindow` expressed in seconds.
///
/// This deduction determines the value of the GRIB `lengthOfTimeWindow`
/// (in seconds) based on the parameter dictionary key `lengthOfTimeWindow`.
///
/// The deduction follows these rules:
///
/// - If the key `lengthOfTimeWindow` is present in the parameter dictionary,
/// its value is interpreted as **hours** and converted to seconds.
/// - If the key is absent, a **default value of 0xFFFF** is used,
///   which is a common convention for "missing" in GRIB.
///
/// @important
/// This deduction currently relies on **implicit assumptions** about
/// units and defaults that are not explicitly encoded in MARS metadata.
/// These assumptions are documented but not enforced via validation.
///
/// @assumptions
/// - `par::lengthOfTimeWindow` is expressed in **hours**
/// - Default value is **0xFFFF** when the key is missing
///
/// @warning
/// - These assumptions may not be valid for all datasets.
/// - Relying on implicit defaults may lead to non-reproducible GRIB output
///   if upstream conventions change.
///
/// @tparam MarsDict_t Type of the MARS dictionary (unused)
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary (unused)
/// @param[in] par  Parameter dictionary
/// @param[in] opt  Options dictionary (unused)
///
/// @return Returns seconds when provided; otherwise returns the GRIB missing code 0xFFFF.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - access to the parameter dictionary fails
/// - the retrieved value cannot be interpreted as a valid integer
/// - any unexpected error occurs during deduction
///
/// @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: medium]
/// - Make the unit of `lengthOfTimeWindow` explicit instead of assuming hours.
/// - Add explicit validation of allowed ranges and units.
///
/// @note
/// - This deduction does not rely on any pre-existing GRIB header state.
/// - Logging intentionally emits warnings to highlight implicit assumptions.
///

template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_LengthOfTimeWindowInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {


    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Default value in hours
        constexpr long defaultLengthOfTimeWindow = 0xFFFF;  // Missing

        // Big assumption here:
        // - lengthOfTimeWindow is in hours
        if (has(par, "lengthOfTimeWindow")) {
            long lengthOfTimeWindowInHoursVal = get_or_throw<long>(par, "lengthOfTimeWindow");

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`lengthOfTimeWindow` resolved from input dictionaries: value='";
                logMsg += std::to_string(lengthOfTimeWindowInHoursVal) + "' [hours]";
                return logMsg;
            }());

            // Success exit point
            return lengthOfTimeWindowInHoursVal * 3600;  // Convert hours to seconds
        }
        else {

            // Emit DEFAULT log entry
            MARS2GRIB_LOG_DEFAULT([&]() {
                std::string logMsg = "`lengthOfTimeWindow` defaulted to 'MISSING': value='0xFFFF'";
                return logMsg;
            }());

            // Success exit point (This is just a bit pattern not seconds, but it's a common convention for "missing" in GRIB)
            return defaultLengthOfTimeWindow;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `lengthOfTimeWindow` from Par dictionary", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

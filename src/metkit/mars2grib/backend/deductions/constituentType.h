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
/// @file constituentType.h
/// @brief Deduction of the constituent (chemical species) type identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **constituent / chemical species identifier** from
/// MARS metadata.
///
/// The deduction retrieves the identifier directly from the MARS
/// dictionary and performs basic numeric validation before exposing
/// the value to the encoding layer.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying explicit and minimal validation logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply semantic inference or defaulting
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or invalid inputs cause immediate failure
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
/// - @ref paramId.h
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
/// @brief Resolve the constituent (chemical species) type identifier from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["chem"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the constituent (chemical species) type
/// identifier by retrieving the mandatory MARS key `chem` and returning
/// its value as a `long`.
///
/// A basic numeric validity check is applied. Only values in the
/// inclusive range `[0, 900]` are accepted. Values outside this range
/// result in a deduction failure.
///
/// No semantic interpretation, normalization, or defaulting is applied.
/// The meaning of the identifier is defined by upstream MARS/GRIB
/// conventions.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `chem`
/// and conversion to `long`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the constituent type identifier is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved constituent (chemical species) type identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `chem` is missing, cannot be converted to `long`, if the
/// value is outside the accepted range, or if any unexpected error
/// occurs during deduction.
///
/// @note
/// This deduction enforces conservative numeric validation and does
/// not consult chemical metadata tables or GRIB code tables.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ConstituentType_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS constituent type
        long constituentType = get_or_throw<long>(mars, "chem");

        // Validate
        if (constituentType < 0 || constituentType > 900) {
            throw Mars2GribDeductionException(
                "Invalid `constituentType`: value='" + std::to_string(constituentType) + "'", Here());
        }

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`constituentType` resolved from input dictionaries: value='";
            logMsg += std::to_string(constituentType);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return constituentType;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `constituentType` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file level.h
/// @brief Deduction of the vertical level identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **vertical level identifier** from MARS metadata.
///
/// The deduction retrieves the level identifier directly from the
/// MARS dictionary and exposes it to the encoding layer without
/// transformation or interpretation.
///
/// The semantic interpretation of the level value (e.g. pressure level,
/// model level index) depends on the associated level type (`levtype`)
/// and is handled by downstream encoding logic.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying minimal, explicit deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply inference, normalization, or defaulting
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
/// - @ref levelEncoding.h
///
/// Related deductions:
/// - @ref levtype.h
/// - @ref pvArray.h
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
/// @brief Resolve the vertical level identifier from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["levelist"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the vertical level identifier by retrieving
/// the mandatory MARS key `levelist` and returning its value as a
/// `long`.
///
/// No semantic interpretation or validation of the level value is
/// performed. The meaning of the level identifier depends on the
/// associated level type (`levtype`) and is handled elsewhere.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to
/// `levelist` and conversion to `long`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the level identifier is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved vertical level identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `levelist` is missing, cannot be retrieved as a
/// `long`, or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction performs presence-only validation and does not
/// consult level definition tables or apply unit conversions.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_Level_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS level identifier
        long marsLevelistVal = get_or_throw<long>(mars, "levelist");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`level` resolved from input dictionaries: value='";
            logMsg += std::to_string(marsLevelistVal) + "'";
            return logMsg;
        }());

        // Success exit point
        return marsLevelistVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `levelist` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

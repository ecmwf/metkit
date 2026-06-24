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
/// @file centre.h
/// @brief Deduction of the GRIB `centre` identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **GRIB centre identifier** from MARS metadata.
///
/// The deduction retrieves the originating centre identifier directly
/// from the MARS dictionary and exposes it to the encoding layer without
/// transformation.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying minimal, explicit deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply normalization or defaulting unless explicitly defined
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
/// - @ref originEncoding.h
///
/// Related deductions:
/// - @ref subCentre.h
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
/// @brief Resolve the GRIB `centre` identifier from MARS metadata.
///
/// @section Deduction contract
/// - Reads: `mars["origin"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the GRIB `centre` identifier by retrieving the
/// mandatory MARS key `origin` and returning its value verbatim.
///
/// No normalization, translation, or defaulting is applied at this stage.
/// Any semantic interpretation or mapping to numeric GRIB centre codes
/// must be handled by downstream encoding logic.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `origin`
/// and conversion to `std::string`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the originating centre identifier is read.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The originating centre identifier as provided by MARS.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `origin` is missing, cannot be retrieved as a string,
/// or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction enforces presence-only validation and does not
/// consult GRIB centre code tables.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Centre_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS origin
        std::string origin = get_or_throw<std::string>(mars, "origin");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`centre` resolved from input dictionaries: value='";
            logMsg += origin + "'";
            return logMsg;
        }());

        // Success exit point
        return origin;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `origin` from input dictionaries", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file laplacianOperator.h
/// @brief Deduction of the Laplacian operator coefficient.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **Laplacian operator coefficient** from the parameter
/// dictionary.
///
/// The deduction retrieves the coefficient explicitly from user-provided
/// parameters and exposes it to the encoding layer without transformation
/// or interpretation.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying minimal, explicit deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply inference or defaulting
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
/// - @ref packingEncoding.h
///
/// Related deductions:
/// - @ref bitsPerValue.h
/// - @ref subSetTrunc.h
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
/// @brief Resolve the Laplacian operator coefficient from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `par["laplacianOperator"]`
/// - Writes: none
/// - Side effects: none (no logging)
/// - Failure mode: throws
///
/// This deduction resolves the Laplacian operator coefficient by retrieving
/// the mandatory parameter dictionary key `laplacianOperator` and returning
/// its value as a `double`.
///
/// The value is taken verbatim from the parameter dictionary and overrides
/// any implicit or default behavior. No validation beyond type conversion
/// is performed.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary (unused by this deduction).
///
/// @tparam ParDict_t
/// Type of the parameter dictionary. Must support keyed access to
/// `laplacianOperator` and conversion to `double`.
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary (unused).
///
/// @param[in] par
/// Parameter dictionary from which the Laplacian operator coefficient
/// is resolved.
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved Laplacian operator coefficient.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `laplacianOperator` is missing, cannot be converted to
/// `double`, or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction performs presence-only validation and does not
/// attempt to infer or normalize the coefficient value.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
double resolve_LaplacianOperator_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory Laplacian operator coefficient
        double laplacianOperator = get_or_throw<double>(par, "laplacianOperator");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`laplacianOperator` resolved from input dictionaries: value='";
            logMsg += std::to_string(laplacianOperator);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return laplacianOperator;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `laplacianOperator` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

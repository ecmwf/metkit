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
/// @file modelErrorType.h
/// @brief Deduction of the model-error type identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **model-error type identifier** (`modelErrorType`) from
/// the parameter dictionary.
///
/// The value is not derivable from MARS alone. It must be supplied via
/// the parameter dictionary by the upstream tool, typically read from
/// the input GRIB1 handle being re-encoded.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - enforcing deterministic resolution rules
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply heuristic or data-driven inference
/// - do NOT validate against GRIB code tables unless explicitly required
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or invalid inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref modelErrorEncoding.h
///
/// Related deductions:
/// - @ref componentIndex.h
/// - @ref numberOfComponents.h
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <string>

#include "eckit/log/Log.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the model-error type identifier.
///
/// @section Deduction contract
/// - Reads: `par["modelErrorType"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction retrieves the model-error type identifier from the
/// parameter dictionary.
///
/// The value is treated as mandatory: it cannot be derived from MARS
/// metadata alone and must be supplied by the upstream tool that
/// populates the parameter dictionary (typically read from the input
/// GRIB1 handle being re-encoded).
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary (unused).
///
/// @tparam ParDict_t
/// Type of the parameter dictionary. Must provide the key
/// `modelErrorType`.
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary (unused).
///
/// @param[in] par
/// Parameter dictionary from which the model-error type is retrieved.
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The model-error type identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `modelErrorType` is missing from the parameter dictionary,
/// cannot be converted to `long`, or if any unexpected error occurs
/// during deduction.
///
/// @note
/// This deduction does not infer or default the value. Absence of the
/// key in the parameter dictionary is considered a contract violation
/// by the upstream tool.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ModelErrorType_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory parameter-dictionary modelErrorType
        long modelErrorType = get_or_throw<long>(par, "modelErrorType");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`modelErrorType` resolved from input dictionaries: value=";
            logMsg += std::to_string(modelErrorType);
            return logMsg;
        }());

        // Success exit point
        return modelErrorType;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `modelErrorType` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

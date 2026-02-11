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
/// @file activity.h
/// @brief Deduction of the MARS `activity` attribute.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **MARS activity identifier** from input dictionaries.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying minimal deduction logic where required
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply semantic defaults beyond explicit rules
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or malformed inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved via deduction logic from input dictionaries
/// - OVERRIDE: value provided by parameter dictionary overriding deduction logic
///
/// @section References
/// Concept:
/// - @ref destineEncoding.h
///
/// Related deductions:
/// - @ref experiment.h
/// - @ref generation.h
/// - @ref model.h
/// - @ref realization.h
/// - @ref resolution.h
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
/// @brief Resolve the MARS activity identifier from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["activity"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction retrieves the mandatory key `activity` from the MARS
/// dictionary and returns its value as a `std::string`.
///
/// The value is resolved directly from the input dictionaries without
/// semantic interpretation, validation, or defaulting. The meaning of
/// the activity identifier is defined by upstream MARS conventions.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `activity`
/// and conversion to `std::string`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the activity identifier is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved MARS activity identifier as a `std::string`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `activity` is missing, cannot be converted to
/// `std::string`, or if any unexpected error occurs during access.
///
/// @note
/// This deduction follows a fail-fast strategy and emits exactly one
/// RESOLVE log entry on successful execution.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Activity_or_throw(const MarsDict_t& mars, [[maybe_unused]] const ParDict_t& par,
                                      [[maybe_unused]] const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS activity
        std::string marsActivityVal = get_or_throw<std::string>(mars, "activity");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`activity` resolved from input dictionary: value='" + marsActivityVal + "'";
            return logMsg;
        }());

        // Success exit point
        return marsActivityVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `activity` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file offsetToEndOf4DvarWindow.h
/// @brief Deduction of the offset to the end of the 4D-Var analysis window.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **offset to the end of the 4D-Var assimilation window**
/// from input dictionaries.
///
/// The deduction retrieves the offset explicitly from the MARS dictionary.
/// No inference, defaulting, normalization, or validation of temporal
/// semantics is performed.
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or invalid inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved from one or more input dictionaries
///
/// @section References
/// Concept:
/// - @ref analysisEncoding.h
///
/// Related deductions:
/// - @ref lengthOfTimeWindow.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the offset to the end of the 4D-Var analysis window.
///
/// @section Deduction contract
/// - Reads: `mars["anoffset"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the temporal offset between the analysis
/// reference time and the end of the 4D-Var assimilation window.
///
/// The returned value is treated as an opaque numeric quantity. Its unit
/// and interpretation are defined by upstream MARS/IFS conventions and
/// are not interpreted by this deduction.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must provide the key `anoffset`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary from which the offset is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The offset to the end of the 4D-Var analysis window.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `anoffset` is missing, cannot be converted to `long`,
/// or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction assumes that the offset is explicitly provided by
/// MARS and does not attempt any inference or defaulting.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_offsetToEndOf4DvarWindow_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS anoffset
        auto offsetToEndOf4DvarWindow = get_or_throw<long>(mars, "anoffset");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`offsetToEndOf4DvarWindow` resolved from input dictionaries: value='";
            logMsg += std::to_string(offsetToEndOf4DvarWindow) + "'";
            return logMsg;
        }());

        // Success exit point
        return offsetToEndOf4DvarWindow;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `offsetToEndOf4DvarWindow` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

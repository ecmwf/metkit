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
/// @file truncation.h
/// @brief Deduction of the spectral truncation parameter.
///
/// This header defines the deduction responsible for resolving the
/// spectral truncation parameter used in spectral packing
/// configurations.
///
/// The value is obtained from the MARS dictionary when provided.
/// If absent, a deterministic default is applied.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - OVERRIDE: value overridden from input dictionaries
/// - DEFAULT: value defaulted due to missing input
///
/// @section References
/// Concept:
/// - @ref packingEncoding.h
///
/// Related deductions:
/// - @ref bitsPerValue.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <algorithm>
#include <regex>
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB spectral truncation parameter.
///
/// This deduction resolves the spectral truncation parameter
/// used to define a reduced set of spectral coefficients.
///
/// Resolution rules:
/// - If `mars::truncation` is present and valid, its value is used directly.
/// - If `mars::truncation` is absent, the value defaults explicitly to the
///       smaller of the MARS truncation (`mars::truncation`) and a fixed maximum of 20.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary providing `truncation` for defaulting and validation
/// @param[in] par  Parameter dictionary (unused)
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved spectral subset truncation value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// - If an unexpected error occurs during dictionary access
/// - If `mars::truncation` is provided but is invalid (e.g., negative)
/// - If MARS truncation is invalid when needed for defaulting
///
/// @note
/// This deduction is fully deterministic and does not depend on
/// any pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_Truncation_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    static const std::regex tGridRegex("^T(0|[1-9][0-9]*)$");

    try {

        // truncation must not be larger than any pentagonalResolutionParameter
        // NOTE: Mars keyword truncation is equivalent to pentagonalResolutionParameter{J,K,M}
        //       At ECMWF we cannot produce spherical harmonics with different values for J/K/M

        const long truncation = [&]() {
            if (get_or_throw<bool>(opt, "skipSection3")) {
                const std::string grid = get_or_throw<std::string>(mars, "grid");
                std::smatch tGridMatch;
                if (!std::regex_match(grid, tGridMatch, tGridRegex) || tGridMatch.size() != 2) {
                    throw Mars2GribDeductionException("Cannot extract truncation from MARS grid '" + grid + "'",
                                                      Here());
                }
                return std::stol(tGridMatch[1].str());
            }
            return get_or_throw<long>(mars, "truncation");
        }();

        if (truncation < 0) {
            throw Mars2GribDeductionException(
                "Invalid MARS truncation: value='" + std::to_string(truncation) + "' is negative", Here());
        }
        return truncation;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `truncation` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

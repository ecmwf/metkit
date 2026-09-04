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
/// @file subSetTrunc.h
/// @brief Deduction of the spectral subset truncation parameter.
///
/// This header defines the deduction responsible for resolving the
/// spectral subset truncation parameter used in spectral packing
/// configurations.
///
/// The value is obtained from the parameter dictionary when provided.
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
#include <string>

#include "eckit/geo/Grid.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB spectral subset truncation parameter.
///
/// This deduction resolves the spectral subset truncation parameter
/// used to define a reduced set of spectral coefficients.
///
/// Resolution rules:
/// - If `par::subSetTruncation` is present and valid, its value is used directly.
/// - If `par::subSetTruncation` is absent, the value defaults explicitly to the
///       smaller of the MARS truncation (`mars::truncation`) and a fixed maximum of 20.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary providing `truncation` for defaulting and validation
/// @param[in] par  Parameter dictionary; may contain `subSetTruncation`
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved spectral subset truncation value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// - If an unexpected error occurs during dictionary access
/// - If `par::subSetTruncation` is provided but exceeds the MARS truncation or is negative
/// - If MARS truncation is invalid when needed for defaulting
///
/// @note
/// This deduction is fully deterministic and does not depend on
/// any pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SubSetTruncation_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // subSetTruncation must not be larger than any pentagonalResolutionParameter
        // NOTE: Mars keyword truncation is equivalent to pentagonalResolutionParameter{J,K,M}
        //       At ECMWF we cannot produce spherical harmonics with different values for J/K/M

        const long truncation = [&]() {
            if (get_or_throw<bool>(opt, "skipSection3")) {
                const std::string grid           = get_or_throw<std::string>(mars, "grid");
                const eckit::geo::Grid* gridSpec = eckit::geo::GridFactory::make_from_string(grid);
                return static_cast<long>(gridSpec->truncation());
            }
            return get_or_throw<long>(mars, "truncation");
        }();

        if (truncation < 0) {
            throw Mars2GribDeductionException(
                "Invalid MARS truncation: value='" + std::to_string(truncation) + "' is negative", Here());
        }

        if (has(par, "subSetTruncation")) {

            // Retrieve subSetTruncation from parameter dictionary
            long subSetTrunc = get_or_throw<long>(par, "subSetTruncation");

            // Validate subSetTruncation
            if (subSetTrunc < 0) {
                throw Mars2GribDeductionException(
                    "Invalid `subSetTruncation`: value='" + std::to_string(subSetTrunc) + "' is negative", Here());
            }
            if (subSetTrunc > truncation) {
                throw Mars2GribDeductionException("Invalid `subSetTruncation`: value='" + std::to_string(subSetTrunc) +
                                                      "' exceeds MARS truncation='" + std::to_string(truncation) + "'",
                                                  Here());
            }

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                return "`subSetTruncation` overridden from parameter dictionary: value='" +
                       std::to_string(subSetTrunc) + "'";
            }());

            // Success exit point
            return subSetTrunc;
        }
        else {
            // Note: This logic for default subSetTruncation reflects the behaviour of IFS
            const long defaultSubSetTruncation = truncation >= 213 ? 20L : std::min(10L, truncation);

            // Emit DEFAULT log entry for defaulting
            MARS2GRIB_LOG_DEFAULT([&]() {
                return "`subSetTruncation` defaulted: value='" + std::to_string(defaultSubSetTruncation) + "'";
            }());

            // Success exit point
            return defaultSubSetTruncation;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `subSetTruncation` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

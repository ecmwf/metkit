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
/// @file channel.h
/// @brief Deduction of the instrument channel identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **instrument channel identifier** from MARS metadata.
///
/// The deduction retrieves the channel identifier directly from the
/// MARS dictionary and exposes it to the encoding layer without
/// transformation or interpretation.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying minimal, explicit deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply inference, defaulting, or consistency checks
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
/// - @ref satelliteEncoding.h
///
/// Related deductions:
/// - @ref instrumentType.h
/// - @ref satelliteNumber.h
/// - @ref satelliteSeries.h
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
/// @brief Resolve the instrument channel identifier from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["channel"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the instrument channel identifier by retrieving
/// the mandatory MARS key `channel` and returning its value as a `long`.
///
/// No semantic interpretation, normalization, or validation is performed
/// beyond basic type conversion. The meaning of the channel identifier is
/// defined by upstream metadata conventions.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `channel`
/// and conversion to `long`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the channel identifier is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved instrument channel identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `channel` is missing, cannot be converted to `long`,
/// or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction performs presence-only validation and does not
/// consult instrument metadata or GRIB tables.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_Channel_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS channel
        long channel = get_or_throw<long>(mars, "channel");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`channel` resolved from input dictionaries: value='";
            logMsg += std::to_string(channel);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return channel;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `channel` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

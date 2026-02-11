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
/// @file satelliteSeries.h
/// @brief Deduction of the GRIB `satelliteSeries` identifier.
///
/// This header defines the deduction responsible for resolving the
/// GRIB `satelliteSeries` key used in satellite-based products.
///
/// The value is not inferable from MARS metadata and must be provided
/// explicitly via the parameter dictionary.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values
/// - apply defaults or fallbacks
/// - validate against GRIB code tables
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref satelliteEncoding.h
///
/// Related deductions:
/// - @ref satelliteNumber.h
/// - @ref instrumentType.h
/// - @ref channel.h
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
/// @brief Resolve the GRIB `satelliteSeries` identifier.
///
/// @section Deduction contract
/// - Reads: `par["satelliteSeries"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction retrieves the mandatory `satelliteSeries` entry from
/// the parameter dictionary and returns it verbatim.
///
/// No defaulting, inference, or semantic validation is performed.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary (unused).
///
/// @tparam ParDict_t
/// Type of the parameter dictionary. Must provide `satelliteSeries`.
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary (unused).
///
/// @param[in] par
/// Parameter dictionary providing the satellite series identifier.
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// Satellite series identifier to be encoded in the GRIB message.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `satelliteSeries` is missing, cannot be retrieved as a
/// `long`, or if any unexpected error occurs.
///
/// @note
/// This deduction is deterministic and does not depend on any
/// pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SatelliteSeries_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory satellite series identifier from parameter dictionary
        long marsSatelliteSeriesVal = get_or_throw<long>(par, "satelliteSeries");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`satelliteSeries` resolved from parameter dictionary: value='";
            logMsg += std::to_string(marsSatelliteSeriesVal);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return marsSatelliteSeriesVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `satelliteSeries` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

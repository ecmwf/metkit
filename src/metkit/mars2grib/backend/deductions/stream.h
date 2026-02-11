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
/// @file stream.h
/// @brief Deduction of the GRIB `stream` identifier.
///
/// This header defines the deduction responsible for resolving the
/// GRIB `stream` key used to describe the MARS data stream associated
/// with the encoded product.
///
/// The value is not inferred or transformed and must be provided
/// explicitly by the MARS dictionary.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values
/// - apply defaults or fallbacks
/// - validate stream values against controlled vocabularies
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref marsEncoding.h
///
/// Related deductions:
/// - @ref class.h
/// - @ref expver.h
/// - @ref type.h
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
/// @brief Resolve the GRIB `stream` key.
///
/// This deduction retrieves the value of the MARS key `stream`
/// from the input MARS dictionary and exposes it directly
/// as the GRIB `stream` identifier.
///
/// The value is treated as mandatory and no inference,
/// defaulting, or validation is performed.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary (unused)
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary; must contain the key `stream`
/// @param[in] par  Parameter dictionary (unused)
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved MARS stream identifier
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - `stream` is missing from the MARS dictionary
/// - the value cannot be retrieved as `std::string`
/// - any unexpected error occurs during deduction
///
/// @note
/// This deduction is fully deterministic and does not depend on
/// any pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Stream_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory stream from Mars dictionary
        std::string marsStreamVal = get_or_throw<std::string>(mars, "stream");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`stream` resolved from input dictionaries: value='";
            logMsg += marsStreamVal;
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return marsStreamVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `stream` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

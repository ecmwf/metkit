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
/// @file systemNumber.h
/// @brief Deduction of the GRIB wave system identifier.
///
/// This header defines the deduction responsible for resolving the
/// GRIB wave system identifier used in wave-related products.
///
/// The value is obtained directly from MARS metadata and is treated
/// as mandatory.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values
/// - apply defaults or fallbacks
/// - validate semantic correctness
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref longrangeEncoding.h
///
/// Related deductions:
/// - @ref methodNumber.h
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
/// @brief Resolve the GRIB wave system identifier.
///
/// This deduction resolves the wave system identifier by retrieving
/// the mandatory MARS key `system`.
///
/// Resolution rules:
/// - `mars::system` MUST be present
/// - No defaulting or inference is applied
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary (unused)
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary; must contain `system`
/// @param[in] par  Parameter dictionary (unused)
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved wave system identifier
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the system identifier cannot be resolved
///
/// @note
/// This deduction is deterministic and does not rely on any
/// pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SystemNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory systemNumber from MARS dictionary
        auto systemNumber = get_or_throw<long>(mars, "system");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`systemNumber` resolved from input dictionaries: value='";
            logMsg += std::to_string(systemNumber);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return systemNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `systemNumber` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

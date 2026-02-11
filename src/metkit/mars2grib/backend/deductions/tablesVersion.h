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
/// @file tablesVersion.h
/// @brief Deduction of the GRIB tables version identifiers.
///
/// This header defines the deductions responsible for resolving the
/// GRIB tables version identifiers used during GRIB encoding.
///
/// Two resolution strategies are provided:
/// - automatic resolution of the latest tables version supported by ecCodes
/// - explicit user override via the parameter dictionary
///
/// Deductions:
/// - extract values from input dictionaries or runtime environment
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values
/// - apply silent fallbacks
/// - validate semantic correctness against GRIB specifications
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained directly from input dictionaries or runtime
/// - OVERRIDE: value explicitly provided by the user
///
/// @section References
/// Concept:
/// - @ref tablesEncoding.h
///
/// Related deductions:
/// - @ref localTablesVersion.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the latest GRIB tables version supported by ecCodes.
///
/// This deduction resolves the GRIB `tablesVersionLatest` identifier by
/// querying the ecCodes runtime environment.
///
/// Resolution rules:
/// - the value is obtained directly from an ecCodes GRIB2 sample
/// - no MARS or parameter input is used
/// - no defaulting or inference is applied
///
/// @tparam MarsDict_t Type of the MARS dictionary (unused)
/// @tparam ParDict_t  Type of the parameter dictionary (unused)
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary (unused)
/// @param[in] par  Parameter dictionary (unused)
/// @param[in] opt  Options dictionary (unused)
///
/// @return The latest GRIB tables version supported by ecCodes
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the value cannot be resolved from the runtime environment
///
/// @note
/// The returned value is deterministic for a given ecCodes installation.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TablesVersionLatest_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory tablesVersionLatest from ecCodes runtime
        long tablesVersionLatestVal = metkit::codes::codesHandleFromSample("GRIB2")->getLong("tablesVersionLatest");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`tablesVersionLatest` resolved from input dictionaries: value='";
            logMsg += std::to_string(tablesVersionLatestVal);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return tablesVersionLatestVal;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `tablesVersionLatest` from input dictionaries", Here()));
    }
};

///
/// @brief Resolve a user-defined GRIB tables version.
///
/// This deduction resolves the GRIB `tablesVersion` identifier from the
/// parameter dictionary.
///
/// Resolution rules:
/// - `par::tablesVersion` MUST be present
/// - the value is treated as an explicit user override
/// - no validation against ecCodes capabilities is performed
///
/// @tparam MarsDict_t Type of the MARS dictionary (unused)
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary (unused)
/// @param[in] par  Parameter dictionary; must contain `tablesVersion`
/// @param[in] opt  Options dictionary (unused)
///
/// @return The GRIB tables version explicitly requested by the user
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the value cannot be resolved
///
/// @note
/// Callers requiring strict reproducibility must ensure compatibility
/// with the ecCodes runtime environment.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TablesVersionCustom_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory tablesVersion from parameter dictionary
        long tablesVersionCustomVal = get_or_throw<long>(par, "tablesVersion");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`tablesVersion` overridden from parameter dictionary: value='";
            logMsg += std::to_string(tablesVersionCustomVal);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return tablesVersionCustomVal;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `tablesVersion` from input dictionaries", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

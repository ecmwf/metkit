/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file type.h
 * @brief Deduction of the MARS `type` identifier.
 *
 * This header defines the deduction responsible for resolving the
 * MARS `type` key used to classify the nature of a field
 * (e.g. analysis, forecast, ensemble member).
 *
 * The value is retrieved directly from the MARS dictionary and is
 * treated as mandatory.
 *
 * Deductions:
 * - extract values from input dictionaries
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer missing values
 * - apply defaults or fallbacks
 * - validate semantic correctness of the returned value
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: value obtained directly from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref marsEncoding.h
 *
 * Related deductions:
 *   - @ref class.h
 *   - @ref stream.h
 *   - @ref expver.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the MARS `type` identifier.
 *
 * This deduction resolves the MARS `type` key from the MARS dictionary.
 *
 * Resolution rules:
 * - `mars::type` MUST be present
 * - the value is retrieved verbatim as a string
 * - no inference, defaulting, or validation is applied
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary; must contain `type`
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved MARS `type` identifier
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If the value cannot be resolved
 *
 * @note
 * The returned value is not interpreted by this deduction and is
 * assumed to follow MARS conventions.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Type_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory type from MARS dictionary
        std::string marsTypeVal = get_or_throw<std::string>(mars, "type");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`type` resolved from input dictionaries: value='";
            logMsg += marsTypeVal;
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return marsTypeVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Failed to resolve `type` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
 * @file subCentre.h
 * @brief Deduction of the GRIB `subCentre` identifier.
 *
 * This header defines the deduction responsible for resolving the
 * GRIB `subCentre` key, which identifies the originating sub-centre
 * within a GRIB-producing centre.
 *
 * The value is obtained from the parameter dictionary when provided.
 * If absent, the deduction applies an explicit and deterministic
 * default according to GRIB conventions.
 *
 * Deductions:
 * - extract values from input dictionaries
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer missing values from MARS metadata
 * - apply implicit or hidden defaults
 * - validate against official GRIB code tables
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: value obtained or defaulted from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref originEncoding.h
 *
 * Related deductions:
 *   - @ref centre.h
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
 * @brief Resolve the GRIB `subCentre` key.
 *
 * This deduction resolves the GRIB `subCentre` identifier using the
 * parameter dictionary.
 *
 * Resolution rules:
 * - If `par::subCentre` is present, its value is used directly.
 * - If `par::subCentre` is absent, the value defaults explicitly to `0`,
 *   corresponding to the GRIB convention for an unspecified sub-centre.
 *
 * No inference from MARS metadata is performed.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; may contain `subCentre`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved GRIB `subCentre` value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If an unexpected error occurs during dictionary access
 *
 * @note
 * This deduction is fully deterministic and does not depend on
 * any pre-existing GRIB header state.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SubCentre_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve optional subCentre from parameter dictionary
        long subCentre = get_opt<long>(par, "subCentre").value_or(0L);

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`subCentre` resolved from input dictionaries: value='";
            logMsg += std::to_string(subCentre);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return subCentre;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `subCentre` from input dictionaries", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

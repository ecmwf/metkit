/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/enableOptions.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `bitsPerValue` packing parameter.
 *
 * This deduction resolves the GRIB `bitsPerValue` key used by the
 * Data Representation Section to control numeric packing precision.
 *
 * The value is obtained **explicitly** from the parameter dictionary (`par`)
 * and is treated as **mandatory** for this deduction.
 *
 * No implicit defaulting, fallback, or inference is performed.
 * If the key is missing or the value is invalid, the deduction fails.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `bitsPerValue`.
 * All validation, range checking, and future policy logic related to
 * this GRIB key must be implemented here.
 *
 * @section Semantics
 * - Input source: parameter dictionary (`par`)
 * - Resolution type: mandatory
 * - Validation: numeric range check
 * - Defaulting: none
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; must contain `bitsPerValue`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved number of bits per value to be used for GRIB packing.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `bitsPerValue` is missing from the parameter dictionary
 *         - the value is non-positive
 *         - the value exceeds the supported upper bound
 *         - any unexpected error occurs during deduction
 *
 * @validation
 * The following constraints are enforced:
 * - `bitsPerValue > 0`
 * - `bitsPerValue <= 64`
 *
 * @warning
 * No attempt is made to infer or adjust `bitsPerValue` based on data range,
 * parameter metadata, or encoding templates.
 * Supplying inconsistent values may lead to inefficient or invalid GRIB
 * packing.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: extensibility][prio: low]
 * - Introduce a table-driven or policy-based default mapping for
 *   `bitsPerValue` when explicit values are not provided.
 * - Align defaults with parameter metadata or encoding profiles if required.
 *
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_BitsPerValue_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get bits per value
        long bitsPerValue;
        if (has(par, "bitsPerValue")) {
            bitsPerValue = get_or_throw<long>(par, "bitsPerValue");
        }
        else {
            /// @todo we can add a default table-based mapping here in future if needed
            throw Mars2GribDeductionException("Missing mandatory key `bitsPerValue` in Par dictionary", Here());
        }

        // Validate bits per value
        if (bitsPerValue <= 0 || bitsPerValue > 64) {
            std::string errMsg = "Invalid `bitsPerValue` in Par dictionary: ";
            errMsg += "actual='" + std::to_string(bitsPerValue) + "'";
            throw Mars2GribDeductionException(errMsg, Here());
        }

        // Logging of the mars::model
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`bitsPerValue`: mapped from `par::bitsPerValue`: actual='";
            logMsg += std::to_string(bitsPerValue) + "'";
            return logMsg;
        }());

        // Get the mars.freq
        return bitsPerValue;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `bitsPerValue` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

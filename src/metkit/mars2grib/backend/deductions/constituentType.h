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
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the constituent (chemical species) type identifier from the MARS dictionary.
 *
 * This deduction retrieves the value associated with the key `chem`
 * from the MARS dictionary (`mars`). The value is expected to be convertible
 * to a `long` and is treated as mandatory.
 *
 * The resolved value represents a constituent or chemical species identifier
 * (e.g. aerosol, trace gas, or chemical component) used in atmospheric or
 * wave-related products. The numerical meaning of this identifier follows
 * upstream MARS/GRIB conventions and is not interpreted further by this
 * deduction.
 *
 * A basic validity check is performed on the retrieved value. Currently,
 * only values in the inclusive range `[0, 900]` are accepted. Values outside
 * this range are considered invalid and result in a domain-specific exception.
 *
 * The resolved value is logged for diagnostic and traceability purposes.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `chem`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the constituent type identifier is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The constituent (chemical species) type identifier resolved from the
 *   MARS dictionary, returned as a `long`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `chem` is not present in the MARS dictionary,
 *   - the associated value cannot be converted to `long`,
 *   - the value is outside the accepted range `[0, 900]`,
 *   - any unexpected error occurs during dictionary access.
 *
 * @note
 *   The current validity range check is intentionally conservative and may
 *   be refined or extended once the full set of supported constituent
 *   identifiers is formally specified.
 *
 * @note
 *   This deduction assumes that the constituent type identifier is explicitly
 *   provided by the MARS dictionary and does not attempt any inference or
 *   defaulting beyond basic validation.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested exception
 *   propagation to ensure that error provenance is preserved across API
 *   boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ConstituentType_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.chem
        auto constituentType = get_or_throw<long>(mars, "chem");

        // TODO MIVAL: Validate
        if (constituentType < 0 || constituentType > 900) {
            throw Mars2GribDeductionException(
                "Invalid value for `constituentType` in Mars dictionary: " + std::to_string(constituentType), Here());
        }

        // Logging of the channel
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "constituentType: looked up from Mars dictionary with value: ";
            logMsg += std::to_string(constituentType);
            return logMsg;
        }());

        // Return value
        return constituentType;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `constituentType` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

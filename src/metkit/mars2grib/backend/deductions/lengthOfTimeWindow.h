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

#include <exception>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `lengthOfTimeWindow` expressed in seconds.
 *
 * This deduction determines the value of the GRIB `lengthOfTimeWindow`
 * (in seconds) based on the MARS key `lengthOfTimeWindow`.
 *
 * The deduction follows these rules:
 *
 * - If the key `lengthOfTimeWindow` is present in the MARS dictionary,
 *   its value is interpreted as **hours** and converted to seconds.
 * - If the key is absent, a **default value of 12 hours** is assumed
 *   and converted to seconds.
 *
 * @important
 * This deduction currently relies on **implicit assumptions** about
 * units and defaults that are not explicitly encoded in MARS metadata.
 * These assumptions are documented but not enforced via validation.
 *
 * @assumptions
 * - `mars::lengthOfTimeWindow` is expressed in **hours**
 * - Default value is **12 hours** when the key is missing
 *
 * @warning
 * - These assumptions may not be valid for all datasets.
 * - Relying on implicit defaults may lead to non-reproducible GRIB output
 *   if upstream conventions change.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary; may contain `lengthOfTimeWindow`
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return Length of the time window expressed in **seconds**
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - access to the MARS dictionary fails
 *         - the retrieved value cannot be interpreted as a valid integer
 *         - any unexpected error occurs during deduction
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: medium]
 * - Make the unit of `lengthOfTimeWindow` explicit instead of assuming hours.
 * - Replace the hard-coded default (12 hours) with a table-driven or
 *   specification-based default.
 * - Add explicit validation of allowed ranges and units.
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - Logging intentionally emits warnings to highlight implicit assumptions.
 */

template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_LengthOfTimeWindowInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {


    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Default value in hours
        constexpr long defaultLengthOfTimeWindowHours = 0xFFFF;  // Missing

        // Big assumption here:
        // - lengthOfTimeWindow is in hours
        if (has(par, "lengthOfTimeWindow")) {
            long lengthOfTimeWindowVal = get_or_throw<long>(par, "lengthOfTimeWindow");

            // Logging of the par::lengthOfTimeWindow
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "WARNING: ";
                logMsg += "`lengthOfTimeWindow` deduced from `par::lengthOfTimeWindow`";
                logMsg += " is assumed to be in hours. Value retrieved: ";
                logMsg += std::to_string(lengthOfTimeWindowVal) + " hours.";
                return logMsg;
            }());

            return lengthOfTimeWindowVal * 3600;  // Convert hours to seconds
        }
        else {

            // Logging of the par::lengthOfTimeWindow
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "WARNING: ";
                logMsg += "`lengthOfTimeWindow` is missing!";
                return logMsg;
            }());

            return defaultLengthOfTimeWindowHours * 3600;  // Convert hours to seconds
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `lengthOfTimeWindow` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

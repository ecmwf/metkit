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

#include <algorithm>
#include <string>
#include <vector>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/enableOptions.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::validation {

/**
 * @brief Validate the Local Definition Number in the Local Use Section.
 *
 * This function verifies that, when a GRIB *Local Use Section* is present,
 * its `localDefinitionNumber` matches one of the expected values provided
 * by the caller.
 *
 * The validation is performed **only if** the option `applyChecks` is present
 * in the options dictionary (`opt`) and evaluates to `true`.
 *
 * When enabled, the function:
 * - checks that the Local Use Section is present (`LocalUsePresent != 0`);
 * - reads the key `localDefinitionNumber` from the output dictionary (`out`);
 * - compares it against the list of expected local definition numbers.
 *
 * If the Local Use Section is missing, or if the local definition number does
 * not match any of the expected values, an exception is thrown.
 *
 * Any failure occurring during dictionary access or validation is caught and
 * rethrown as a nested `Mars2GribValidationException` with additional context.
 *
 * @tparam OptDict_t Type of the options dictionary
 * @tparam OutDict_t Type of the output dictionary
 *
 * @param[in] opt Options dictionary; may contain the boolean key `applyChecks`
 * @param[in] out Output dictionary expected to contain the keys
 *                `LocalUsePresent` and `localDefinitionNumber`
 *                when checks are enabled
 * @param[in] expectedLocalDefinitionNumber
 *                List of acceptable Local Definition Numbers
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribValidationException
 *         If:
 *         - checks are enabled and the Local Use Section is not present
 *         - the local definition number does not match any expected value
 *         - required keys are missing
 *         - any error occurs while accessing the dictionaries
 *
 * @note
 * - If `applyChecks` is absent or evaluates to `false`, no validation is performed.
 * - The function returns normally on success and does not produce any output.
 */

template <class OptDict_t, class OutDict_t>
void match_LocalDefinitionNumber_or_throw(const OptDict_t& opt, const OutDict_t& out,
                                          const std::vector<long>& expectedLocalDefinitionNumber) {

    using metkit::mars2grib::utils::checksEnabled;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::joinNumbers;
    using metkit::mars2grib::utils::exceptions::Mars2GribValidationException;

    try {

        if (checksEnabled<OptDict_t, OutDict_t>(opt)) {

            // If Local Use Section is present, check definition number
            if (long hasLocalUseSection = get_or_throw<long>(out, "localUsePresent"); hasLocalUseSection != 0) {

                long actualLocalDefinitionNumber = get_or_throw<long>(out, "localDefinitionNumber");

                // Compare against expected values
                bool match = std::find(expectedLocalDefinitionNumber.begin(), expectedLocalDefinitionNumber.end(),
                                       actualLocalDefinitionNumber) != expectedLocalDefinitionNumber.end();

                // Throw if no match
                if (!match) {
                    std::string errMsg = "Local Definition Number mismatch in Local Use Section: ";
                    errMsg += "actual=" + std::to_string(actualLocalDefinitionNumber);
                    errMsg += ", expected=" + joinNumbers(expectedLocalDefinitionNumber);
                    throw Mars2GribValidationException(errMsg, Here());
                }
            }
            else {
                throw Mars2GribValidationException("Local Use Section not present in the sample", Here());
            }

            // Useful for debugging
            MARS2GRIB_LOG_MATCH("Local Definition Number matches expected values");
        }

        // Exit on success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribValidationException("Unable to validate Local Definition Number in Local Use Section", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::validation

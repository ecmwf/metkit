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
 * @brief Validate the Data Representation Template Number against a set of expected values.
 *
 * This function verifies that the GRIB *Data Representation Template Number*
 * matches one of the expected template numbers provided by the caller.
 *
 * The validation is performed **only if** the option `applyChecks` is present
 * in the options dictionary (`opt`) and evaluates to `true`.
 *
 * When enabled, the function reads the key `dataRepresentationTemplateNumber`
 * from the output dictionary (`out`) and compares it against the list of
 * expected values supplied in `expectedDataRepresentationTemplateNumber`.
 *
 * If the actual template number does not match any of the expected values,
 * an exception is thrown.
 *
 * Any failure occurring during dictionary access or validation is caught and
 * rethrown as a nested `Mars2GribValidationException` with additional context.
 *
 * @tparam OptDict_t Type of the options dictionary
 * @tparam OutDict_t Type of the output dictionary
 *
 * @param[in] opt Options dictionary; may contain the boolean key `applyChecks`
 * @param[in] out Output dictionary expected to contain the key
 *                `dataRepresentationTemplateNumber` when checks are enabled
 * @param[in] expectedDataRepresentationTemplateNumber
 *                List of acceptable Data Representation Template Numbers
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribValidationException
 *         If:
 *         - `applyChecks` is `true` and the actual template number does not match
 *           any of the expected values
 *         - required keys are missing
 *         - any error occurs while accessing the dictionaries
 *
 * @note
 * - If `applyChecks` is absent or evaluates to `false`, no validation is performed.
 * - The function returns normally on success and does not produce any output.
 */
template <class OptDict_t, class OutDict_t>
void match_DataRepresentationTemplateNumber_or_throw(
    const OptDict_t& opt, const OutDict_t& out, const std::vector<long>& expectedDataRepresentationTemplateNumber) {

    using metkit::mars2grib::utils::checksEnabled;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::joinNumbers;
    using metkit::mars2grib::utils::exceptions::Mars2GribValidationException;

    try {

        if (checksEnabled<OutDict_t>(opt)) {

            // Get the dataRepresentationTemplateNumber
            long actualDataRepresentationTemplateNumber = get_or_throw<long>(out, "dataRepresentationTemplateNumber");

            // Compare against expected values
            bool match =
                std::find(expectedDataRepresentationTemplateNumber.begin(),
                          expectedDataRepresentationTemplateNumber.end(),
                          actualDataRepresentationTemplateNumber) != expectedDataRepresentationTemplateNumber.end();

            // Throw if no match
            if (!match) {
                std::string errMsg = "Data Representation Template Number does not match any of the expected values: ";
                errMsg += "actual=" + std::to_string(actualDataRepresentationTemplateNumber);
                errMsg += ", expected=" + joinNumbers(expectedDataRepresentationTemplateNumber);
                throw Mars2GribValidationException(errMsg, Here());
            }

            // Useful for debugging
            MARS2GRIB_LOG_MATCH("Data Representation Template Number matches expected values");
        }

        // Exit on success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribValidationException("Unable to validate Data Representation Template Number", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::validation

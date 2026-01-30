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

namespace metkit::mars2grib::backend::validation {

/**
 * @brief Validate the DestinE Local Use Section against production status rules.
 *
 * This function verifies that, when a GRIB *Local Use Section* is present,
 * its content is compatible with the expected DestinE conventions.
 *
 * The validation is performed **only if** the option `applyChecks` is present
 * in the options dictionary (`opt`) and evaluates to `true`.
 *
 * When enabled, the function:
 * - checks that the Local Use Section is present (`LocalUsePresent != 0`);
 * - reads the key `productionStatusOfProcessedData` from the output dictionary;
 * - throws an exception if the production status is different from the only
 *   allowed DestinE value (`12`);
 *
 * If the Local Use Section is expected but not present, an exception is raised.
 *
 * Any failure occurring during dictionary access or validation is caught and
 * rethrown as a nested `Mars2GribValidationException` with additional context.
 *
 * @tparam OptDict_t Type of the options dictionary
 * @tparam OutDict_t Type of the output dictionary
 *
 * @param[in] opt Options dictionary; may contain the boolean key `applyChecks`
 * @param[in] out Output dictionary expected to contain the keys
 *                `LocalUsePresent` and `productionStatusOfProcessedData`
 *                when checks are enabled
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribValidationException
 *         If:
 *         - checks are enabled and the Local Use Section is not present
 *         - the production status is not compatible with DestinE rules
 *         - any error occurs while accessing the dictionaries
 *
 * @note
 * - If `applyChecks` is absent or evaluates to `false`, no validation is performed.
 * - The function returns normally on success and does not produce any output.
 */
template <class OptDict_t, class OutDict_t>
void check_DestinELocalSection_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::checksEnabled;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribValidationException;

    try {

        if (checksEnabled<OutDict_t>(opt)) {

            // If Local Use Section is present, validate production status
            if (long hasLocalUseSection = get_or_throw<long>(out, "LocalUsePresent"); hasLocalUseSection != 0) {

                long actualProductionStatusOfProcessedData = get_or_throw<long>(out, "productionStatusOfProcessedData");

                // Throw if no match
                constexpr long kDestinEProductionStatus = 12;
                if (actualProductionStatusOfProcessedData != kDestinEProductionStatus) {
                    std::string errMsg = "Invalid DestinE Local Use Section (wrong productionStatusOfProcessedData): ";
                    errMsg += "actual=" + std::to_string(actualProductionStatusOfProcessedData);
                    errMsg += ", expected=" + std::to_string(kDestinEProductionStatus);
                    throw Mars2GribValidationException(errMsg, Here());
                }
            }
            else {
                throw Mars2GribValidationException("DestinE Local Use Section not allocated in the sample", Here());
            }

            // Useful for debugging
            MARS2GRIB_LOG_CHECK("Validated DestinE Local Use Section");
        }

        // Exit on success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribValidationException("Unable to validate DestinE Local Use Section", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::validation

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

///
/// @brief Verify that the Product Definition Section corresponds to a statistics product.
///
/// This function checks whether the GRIB Product Definition Section (PDS)
/// represents a *statistics product*.
///
/// The validation is performed **only if** the option `applyChecks` is present
/// in the options dictionary (`opt`) and evaluates to `true`.
/// If the option is not present, no validation is performed.
///
/// When enabled, the function verifies that the key `numberOfTimeRanges`
/// is present in the output dictionary (`out`).
///
/// The absence of this field indicates that the Product Definition Section
/// does not describe a statistics product and results in an exception.
///
/// Any failure occurring during dictionary access or validation is caught and
/// rethrown as a nested `Mars2GribValidationException` with additional context.
///
/// @tparam OptDict_t Type of the options dictionary
/// @tparam OutDict_t Type of the output dictionary
///
/// @param[in] opt Options dictionary; may contain the boolean key `applyChecks`
/// @param[in] out Output dictionary expected to contain the key `numberOfTimeRanges`
/// when checks are enabled
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribValidationException
/// If:
/// - checks are enabled and `numberOfTimeRanges` is missing
/// - any error occurs while accessing the dictionaries
///
/// @todo Extend the checks to other relevant keys, e.g. `typeOfStatisticsProcessing`
///
/// @note
/// - If `applyChecks` is absent or evaluates to `false`, no validation is performed.
/// - The function returns normally on success and does not produce any output.
///
template <class OptDict_t, class OutDict_t>
void check_StatisticsProductDefinitionSection_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::checksEnabled;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribValidationException;

    try {

        if (checksEnabled<OutDict_t>(opt)) {

            bool hasNumberOfTimeRanges          = has(out, "numberOfTimeRanges");
            bool hasTypeOfStatisticalProcessing = has(out, "typeOfStatisticalProcessing");

            // Statistics product needs to have numberOfTimeRanges defined in the Product Definition Section
            if (!hasNumberOfTimeRanges || !hasTypeOfStatisticalProcessing) {
                throw Mars2GribValidationException("Product Definition Section is not of Statistics type", Here());
            }

            // Useful for debugging
            MARS2GRIB_LOG_CHECK("Product Definition Section is of Statistics type");
        }

        // Exit point with success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribValidationException("Unable to validate Product Definition Section as Statistics type", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::validation

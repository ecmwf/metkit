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
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/enableOptions.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::validation {

///
/// @brief Check that the GRIB message contains a Local Use Section.
///
/// This function verifies the presence of the GRIB *Local Use Section* based on
/// the runtime configuration provided in the options dictionary.
///
/// The check is performed **only if** the option `applyChecks` is present in
/// the options dictionary (`opt`) and evaluates to `true`.
///
/// When enabled, the function reads the key `LocalUsePresent` from the output
/// dictionary (`out`):
/// - a value of `0` indicates that the Local Use Section is missing and results
/// in an exception;
/// - a non-zero value indicates that the section is present and the check succeeds.
///
/// Any failure occurring during dictionary access or validation is caught and
/// rethrown as a nested `Mars2GribValidationException` with additional context.
///
/// @tparam OptDict_t Type of the options dictionary
/// @tparam OutDict_t Type of the output dictionary
///
/// @param[in] opt Options dictionary; may contain the boolean key `applyChecks`
/// @param[in] out Output dictionary; must contain the key `LocalUsePresent`
/// when checks are enabled
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribValidationException
/// If:
/// - `applyChecks` is `true` and `LocalUsePresent == 0`
/// - required keys are missing
/// - any error occurs while accessing the dictionaries
///
/// @note
/// - If `applyChecks` is absent or evaluates to `false`, no validation is performed.
/// - The function returns normally on success and does not produce any output.
///
template <class OptDict_t, class OutDict_t>
void check_LocalUseSection_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::checksEnabled;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribValidationException;

    try {

        if (checksEnabled<OutDict_t>(opt)) {

            long localUsePresent = get_or_throw<long>(out, "localUsePresent");

            if (localUsePresent == 0) {
                throw Mars2GribValidationException("Local Use Section not present in the sample", Here());
            }

            // Useful for debugging
            MARS2GRIB_LOG_CHECK("Local Use Section is present in the sample");
        }

        // Exit point with success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribValidationException("Unable to validate presence of Local Use Section", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::validation

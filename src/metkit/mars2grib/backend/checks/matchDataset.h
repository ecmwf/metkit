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
/// @brief Validate that the dataset identifier matches an expected value.
///
/// This function checks whether the GRIB output dictionary contains a `dataset`
/// entry matching the expected dataset identifier provided by the caller.
///
/// The validation is performed **only if** the option `applyChecks` is present
/// in the options dictionary (`opt`) and evaluates to `true`.
///
/// When enabled, the function reads the key `dataset` from the output dictionary
/// (`out`) and compares it against `expectedDataset`.
/// A mismatch results in an exception.
///
/// Any failure occurring during dictionary access or validation is caught and
/// rethrown as a nested `Mars2GribValidationException` with additional context.
///
/// @tparam OptDict_t Type of the options dictionary
/// @tparam OutDict_t Type of the output dictionary
///
/// @param[in] opt Options dictionary; may contain the boolean key `applyChecks`
/// @param[in] out Output dictionary expected to contain the key `dataset`
/// when checks are enabled
/// @param[in] expectedDataset Expected dataset identifier
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribValidationException
/// If:
/// - `applyChecks` is `true` and the dataset does not match the expected value
/// - required keys are missing
/// - any error occurs while accessing the dictionaries
///
/// @note
/// - If `applyChecks` is absent or evaluates to `false`, no validation is performed.
/// - The function returns normally on success and does not produce any output.
///

template <class OptDict_t, class OutDict_t>
void match_Dataset_or_throw(const OptDict_t& opt, const OutDict_t& out, const std::string& expectedDataset) {

    using metkit::mars2grib::utils::checksEnabled;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribValidationException;

    try {

        if (checksEnabled<OutDict_t>(opt)) {

            // Get the `dataset` entry (expected in DestinE local use sections)
            std::string actualDataset = get_or_throw<std::string>(out, "dataset");

            // Compare against expected values
            if (actualDataset != expectedDataset) {
                std::string errMsg = "Dataset does not match the expected value: ";
                errMsg += "actual=" + actualDataset + ", expected=" + expectedDataset;
                throw Mars2GribValidationException(errMsg, Here());
            }

            // Useful for debugging
            MARS2GRIB_LOG_MATCH("Dataset matches expected value");
        }

        // Exit on success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribValidationException("Unable to validate dataset from the sample", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::validation

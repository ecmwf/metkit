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
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the forecast valid time from the MARS dictionary using a step expressed in seconds.
///
/// This deduction computes the forecast valid time by retrieving the MARS key
/// `step` and interpreting it as a forecast lead time expressed in hours.
/// The lead time is converted to seconds and added to the reference time
/// to obtain the forecast valid `eckit::DateTime`.
///
/// The conversion follows the conventional MARS interpretation:
/// - `step` is assumed to be expressed in hours,
/// - the corresponding number of seconds is obtained as
/// \f$ \text{step} \times 3600 \f$.
///
/// The resolved forecast lead time (in seconds) is logged for diagnostic
/// and traceability purposes.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary, expected to contain the key `step`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the forecast step is retrieved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The forecast valid time as an `eckit::DateTime`, obtained by adding
/// the forecast step (converted to seconds) to the reference time.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - the key `step` is not present in the MARS dictionary,
/// - the associated value cannot be converted to `long`,
/// - any unexpected error occurs during conversion or time computation.
///
/// @note
/// This deduction assumes that the MARS `step` value is expressed in
/// hours. Alternative units (e.g. minutes or seconds) are not supported.
///
/// @note
/// The reference time to which the forecast step is applied is assumed
/// to be available in the surrounding context. This function does not
/// resolve the reference time itself.
///
/// @note
/// The function follows a fail-fast strategy and uses nested exception
/// propagation to preserve full error provenance across API boundaries.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ForecastTimeInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.step
        long marsStep = get_or_throw<long>(mars, "step");

        // Convert step in seconds (Assumed to be in hours)
        long marsStepInSecondsVal = marsStep * 3600;

        // Logging of the forecastTime
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "forecastTime: deduced from mars dictionary with value: ";
            logMsg += std::to_string(marsStepInSecondsVal) + " [seconds]";
            return logMsg;
        }());

        return marsStepInSecondsVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to compute forecast time", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

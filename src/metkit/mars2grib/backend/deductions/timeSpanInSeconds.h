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

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the time span from the MARS dictionary and convert it to seconds.
 *
 * This deduction retrieves the value associated with the key `timespan`
 * from the MARS dictionary (`mars`). The value is expected to be
 * convertible to a `long` and is treated as mandatory.
 *
 * The retrieved value is interpreted according to standard MARS
 * conventions as a time span expressed in **hours**. It is converted
 * to seconds by applying a fixed scaling factor:
 *
 * \f[
 *   \text{timeSpanInSeconds} = \text{timespan} \times 3600
 * \f]
 *
 * The resolved time span (in seconds) is logged for diagnostic and
 * traceability purposes.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `timespan`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the time span is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The time span expressed in seconds, returned as a `long`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `timespan` is not present in the MARS dictionary,
 *   - the associated value cannot be converted to `long`,
 *   - any unexpected error occurs during dictionary access or conversion.
 *
 * @note
 *   This deduction assumes that the MARS `timespan` value is expressed
 *   in hours. No alternative units (e.g. minutes or seconds) are
 *   currently supported.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested exception
 *   propagation to preserve full error provenance across API boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TimeSpanInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.timespan
        long marsTimespanVal = get_or_throw<long>(mars, "timespan");

        long timeSpanInSeconds = marsTimespanVal * 3600;

        // Logging of the timeSpan
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "timeSpan: deduced from mars dictionary with value: ";
            logMsg += std::to_string(timeSpanInSeconds) + " [seconds]";
            return logMsg;
        }());

        return timeSpanInSeconds;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `timespan` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

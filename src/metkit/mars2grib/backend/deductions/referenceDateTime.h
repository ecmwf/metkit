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
#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/deductions/detail/timeUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the reference date and time from the MARS dictionary.
///
/// This deduction constructs an `eckit::DateTime` object from the MARS
/// dictionary entries `date` and `time`. Both values are treated as
/// mandatory and are expected to be provided in the conventional MARS
/// integer formats:
///
/// - `date`: calendar date encoded as `YYYYMMDD`
/// - `time`: clock time encoded as `HHMMSS`
///
/// The raw integer values are first converted into canonical `eckit::Date`
/// and `eckit::Time` objects using dedicated conversion utilities, and are
/// then combined into a single `eckit::DateTime` instance.
///
/// The resolved date and time are logged for diagnostic and traceability
/// purposes.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary, expected to contain the keys `date` and `time`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the reference date and time are retrieved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @return
/// The reference date and time resolved from the MARS dictionary, returned
/// as an `eckit::DateTime` object.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - either `date` or `time` is missing from the MARS dictionary,
/// - the associated values cannot be converted to `long`,
/// - the integer values do not represent a valid calendar date or time,
/// - any unexpected error occurs during conversion or dictionary access.
///
/// @note
/// The conversion assumes standard MARS integer encodings for date
/// (`YYYYMMDD`) and time (`HHMMSS`). Validation and normalization are
/// delegated to the underlying conversion utilities.
///
/// @note
/// A future enhancement may retrieve date and time as strings and rely
/// on higher-level Metkit parsing utilities for normalization and
/// validation.
///
/// @note
/// This deduction follows a fail-fast strategy and uses nested exception
/// propagation to preserve full error provenance across API boundaries.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
eckit::DateTime resolve_ReferenceDateTime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // TODO MIVAL: get as string and parse/normalize with metkit utilities

        // Get the mars.date and mars.time
        long marsDate = get_or_throw<long>(mars, "date");
        long marsTime = get_or_throw<long>(mars, "time");

        // Convert to canonical format
        eckit::Date date = detail::convert_YYYYMMDD2Date_or_throw(marsDate);
        eckit::Time time = detail::convert_hhmmss2Time_or_throw(marsTime);

        // Logging of the resolution
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "date,time: deduced from mars dictionary with value: ";
            logMsg += std::to_string(marsDate) + "," + std::to_string(marsTime);
            return logMsg;
        }());

        return eckit::DateTime(date, time);
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `date` and `time` from Mars dictionary to deduce `dateTime`", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

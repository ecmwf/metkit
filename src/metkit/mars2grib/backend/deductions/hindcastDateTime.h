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
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/mars2grib/backend/deductions/detail/timeUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the hindcast reference date and time from the MARS dictionary.
///
/// This deduction retrieves the hindcast reference date and time from the
/// MARS dictionary entries `hdate` and `htime` and combines them into an
/// `eckit::DateTime` object.
///
/// The values are expected to follow the standard MARS integer encodings:
/// - `hdate`: calendar date encoded as `YYYYMMDD`
/// - `htime`: clock time encoded as `HHMMSS`
///
/// These fields are typically used for hindcast or reforecast products,
/// where the reference time of the forecast differs from the nominal
/// analysis or forecast reference time.
///
/// The resolved hindcast date and time are logged for diagnostic and
/// traceability purposes.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary, expected to contain the keys `hdate`
/// and `htime`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the hindcast date and time are retrieved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The hindcast reference date and time resolved from the MARS dictionary,
/// returned as an `eckit::DateTime` object.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - either `hdate` or `htime` is missing from the MARS dictionary,
/// - the associated values cannot be converted to `long`,
/// - the integer values do not represent a valid calendar date or time,
/// - any unexpected error occurs during dictionary access or conversion.
///
/// @note
/// This deduction assumes standard MARS integer encodings for hindcast
/// date (`YYYYMMDD`) and time (`HHMMSS`). Validation and normalization
/// are expected to be handled by the underlying conversion utilities.
///
/// @note
/// A future enhancement may retrieve hindcast date and time as strings
/// and rely on higher-level Metkit parsing utilities for improved
/// normalization and validation.
///
/// @note
/// This function follows a fail-fast strategy and uses nested exception
/// propagation to preserve full error provenance across API boundaries.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
eckit::DateTime resolve_HindcastDateTime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // TODO MIVAL: get as string and parse/normalize with metkit utilities

        // Get the mars.date and mars.time
        long marsDate = get_or_throw<long>(mars, "hdate");
        long marsTime = get_opt<long>(mars, "htime").value_or(0);

        // Convert to canonical format
        eckit::Date date = detail::convert_YYYYMMDD2Date_or_throw(marsDate);
        eckit::Time time = detail::convert_hhmmss2Time_or_throw(marsTime);

        // Logging of the resolution
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "hindcast[date,time]: deduced from mars dictionary with value: ";
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
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

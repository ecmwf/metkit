// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file startOfReferencePeriod.h
/// @brief Deduction of the start date and time of the GRIB reference period.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **start of the reference period** (`yearOfStartOfReferencePeriod`,
/// `monthOfStartOfReferencePeriod`, `dayOfStartOfReferencePeriod`,
/// `hourOfStartOfReferencePeriod`, `minuteOfStartOfReferencePeriod`,
/// `secondOfStartOfReferencePeriod`).
///
/// The values are taken verbatim from the misc (parameter) dictionary:
/// - year, month and day are mandatory;
/// - hour, minute and second are optional and default to 0.
///
/// Error handling follows a strict fail-fast strategy:
/// - missing mandatory keys or out-of-range values cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved from input dictionaries
///
/// @section References
/// Concept:
/// - @ref referencePeriodEncoding.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Start date and time of the reference period.
///
struct StartOfReferencePeriod {
    long year;
    long month;
    long day;
    long hour;
    long minute;
    long second;
};

namespace detail {

inline void checkRange_or_throw(const char* key, long value, long min, long max) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (value < min || value > max) {
        throw Mars2GribDeductionException("`" + std::string{key} + "` (" + std::to_string(value) +
                                              ") is out of range [" + std::to_string(min) + ", " + std::to_string(max) +
                                              "]",
                                          Here());
    }
}

}  // namespace detail

///
/// @brief Resolve the start date and time of the reference period.
///
/// @section Deduction contract
/// - Reads (par): `yearOfStartOfReferencePeriod`, `monthOfStartOfReferencePeriod`, `dayOfStartOfReferencePeriod`
///                (mandatory), `hourOfStartOfReferencePeriod`, `minuteOfStartOfReferencePeriod`,
///                `secondOfStartOfReferencePeriod` (optional, default 0)
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// Validation: month in [1, 12], day in [1, 31], hour in [0, 23], minute in [0, 59], second in [0, 59].
/// No cross-validation of the calendar date (e.g. days per month) is performed.
///
/// @tparam MarsDict_t  Type of the MARS dictionary (unused).
/// @tparam ParDict_t   Type of the misc (parameter) dictionary providing the start of the reference period.
/// @tparam OptDict_t   Type of the options dictionary (unused).
///
/// @param[in] mars  MARS dictionary (unused).
/// @param[in] par   Misc dictionary providing the keys listed above.
/// @param[in] opt   Options dictionary (unused).
///
/// @return The resolved start date and time of the reference period.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If a mandatory key is missing, a value is out of range, or any unexpected error occurs during deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
StartOfReferencePeriod resolve_StartOfReferencePeriod_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                               const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)mars;
    (void)opt;

    try {

        StartOfReferencePeriod result{};

        result.year   = get_or_throw<long>(par, "yearOfStartOfReferencePeriod");
        result.month  = get_or_throw<long>(par, "monthOfStartOfReferencePeriod");
        result.day    = get_or_throw<long>(par, "dayOfStartOfReferencePeriod");
        result.hour   = get_opt<long>(par, "hourOfStartOfReferencePeriod").value_or(0L);
        result.minute = get_opt<long>(par, "minuteOfStartOfReferencePeriod").value_or(0L);
        result.second = get_opt<long>(par, "secondOfStartOfReferencePeriod").value_or(0L);

        detail::checkRange_or_throw("monthOfStartOfReferencePeriod", result.month, 1, 12);
        detail::checkRange_or_throw("dayOfStartOfReferencePeriod", result.day, 1, 31);
        detail::checkRange_or_throw("hourOfStartOfReferencePeriod", result.hour, 0, 23);
        detail::checkRange_or_throw("minuteOfStartOfReferencePeriod", result.minute, 0, 59);
        detail::checkRange_or_throw("secondOfStartOfReferencePeriod", result.second, 0, 59);

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`startOfReferencePeriod` resolved from input dictionaries: value='";
            logMsg += std::to_string(result.year) + "-" + std::to_string(result.month) + "-" +
                      std::to_string(result.day) + "T" + std::to_string(result.hour) + ":" +
                      std::to_string(result.minute) + ":" + std::to_string(result.second);
            logMsg += "'";
            return logMsg;
        }());

        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `startOfReferencePeriod` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

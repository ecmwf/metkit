/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file typeOfTimeIncrement.h
/// @brief Public deduction header for `typeOfTimeIncrement`.
///
/// Exposes `resolve_TypeOfTimeIncrement_or_throw`, the designated deduction
/// boundary for GRIB Code Table 4.11 resolution used by ProductTimeSpec.
///
/// The concrete deduction logic is intentionally still undefined. The current
/// implementation fails loudly so callers do not accidentally depend on
/// unspecified behavior while the boundary already exists in the architecture.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve `typeOfTimeIncrement`.
///
/// @section Deduction contract
///   - Reads (MARS): none yet
///   - Reads (par):  none yet
///   - Reads (opt):  none yet
///   - Writes:       none
///   - Side effects: none
///   - Failure mode: throws `Mars2GribDeductionException`
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary (reserved for future logic).
/// @param[in] par   Parameter dictionary (reserved for future logic).
/// @param[in] opt   Options dictionary (reserved for future logic).
///
/// @return The resolved GRIB `tables::TypeOfTimeIntervals` value.
///
/// @note The current implementation is a placeholder. It always returns
/// `tables::TypeOfTimeIntervals::SameStartTimeForecastIncremented` and does not
/// perform any deduction logic.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
metkit::mars2grib::backend::tables::TypeOfTimeIntervals resolve_TypeOfTimeIncrement_or_throw(const MarsDict_t& mars,
                                                                                             const ParDict_t& par,
                                                                                             const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory class from MARS dictionary
        std::string marsClass = get_or_throw<std::string>(mars, "class");

        // Retrieve mandatory type from MARS dictionary
        std::string marsType = get_or_throw<std::string>(mars, "type");

        // Retrieve mandatory stream from MARS dictionary
        std::string marsStream = get_or_throw<std::string>(mars, "stream");

        /// Initialize default
        metkit::mars2grib::backend::tables::TypeOfTimeIntervals result =
            metkit::mars2grib::backend::tables::TypeOfTimeIntervals::SameStartTimeForecastIncremented;

        if (marsClass == "a5" && marsStream == "sttd") {
            result = metkit::mars2grib::backend::tables::TypeOfTimeIntervals::SameForecastTimeStartIncremented;
        }

        if (marsClass == "e6" && (marsStream == "sttd" || marsStream == "stte")) {
            result = metkit::mars2grib::backend::tables::TypeOfTimeIntervals::SameForecastTimeStartIncremented;
        }

        if (marsClass == "od" && marsStream == "mnth") {
            result = metkit::mars2grib::backend::tables::TypeOfTimeIntervals::SameForecastTimeStartIncremented;
        }

        // Success exit point
        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `typeOfTimeIncrement` from input dictionaries", Here()));
    }

    return metkit::mars2grib::backend::tables::TypeOfTimeIntervals::SameStartTimeForecastIncremented;
}

}  // namespace metkit::mars2grib::backend::deductions

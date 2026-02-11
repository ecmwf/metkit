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

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief Type of time intervals.
///
/// This enumeration represents GRIB code values defining how successive
/// time intervals are processed in time-dependent fields.
///
/// The numeric values map **directly** to ecCodes GRIB table 4.11
/// and must not be changed manually.
///
/// @note
/// The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-table representation only**.
/// No policy, defaulting, or deduction logic belongs here.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and all conversion helpers automatically from
/// ecCodes definitions at build time.
///
enum class TypeOfTimeIntervals : long {
    Reserved                                         = 0,
    SameForecastTimeStartIncremented                 = 1,
    SameStartTimeForecastIncremented                 = 2,
    StartIncrementedForecastDecrementedConstantValid = 3,
    StartDecrementedForecastIncrementedConstantValid = 4,
    FloatingSubinterval                              = 5,
    Missing                                          = 255
};

///
/// @brief Convert `TypeOfTimeIntervals` to its canonical name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string name
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is invalid.
///
inline std::string enum2name_TypeOfTimeIntervals_or_throw(TypeOfTimeIntervals value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfTimeIntervals::Reserved:
            return "reserved";
        case TypeOfTimeIntervals::SameForecastTimeStartIncremented:
            return "same-forecast-time-start-incremented";
        case TypeOfTimeIntervals::SameStartTimeForecastIncremented:
            return "same-start-time-forecast-incremented";
        case TypeOfTimeIntervals::StartIncrementedForecastDecrementedConstantValid:
            return "start-incremented-forecast-decremented-constant-valid";
        case TypeOfTimeIntervals::StartDecrementedForecastIncrementedConstantValid:
            return "start-decremented-forecast-incremented-constant-valid";
        case TypeOfTimeIntervals::FloatingSubinterval:
            return "floating-subinterval";
        case TypeOfTimeIntervals::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid TypeOfTimeIntervals enum value", Here());
    }
}


///
/// @brief Convert a canonical name to `TypeOfTimeIntervals`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the name is not recognized.
///
inline TypeOfTimeIntervals name2enum_TypeOfTimeIntervals_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "reserved")
        return TypeOfTimeIntervals::Reserved;
    else if (name == "same-forecast-time-start-incremented")
        return TypeOfTimeIntervals::SameForecastTimeStartIncremented;
    else if (name == "same-start-time-forecast-incremented")
        return TypeOfTimeIntervals::SameStartTimeForecastIncremented;
    else if (name == "start-incremented-forecast-decremented-constant-valid")
        return TypeOfTimeIntervals::StartIncrementedForecastDecrementedConstantValid;
    else if (name == "start-decremented-forecast-incremented-constant-valid")
        return TypeOfTimeIntervals::StartDecrementedForecastIncrementedConstantValid;
    else if (name == "floating-subinterval")
        return TypeOfTimeIntervals::FloatingSubinterval;
    else if (name == "missing")
        return TypeOfTimeIntervals::Missing;
    else
        throw Mars2GribTableException("Invalid TypeOfTimeIntervals name: '" + name + "'", Here());
}


///
/// @brief Convert a numeric GRIB code to `TypeOfTimeIntervals`.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the value is not defined in the GRIB table.
///
inline TypeOfTimeIntervals long2enum_TypeOfTimeIntervals_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfTimeIntervals::Reserved;
        case 1:
            return TypeOfTimeIntervals::SameForecastTimeStartIncremented;
        case 2:
            return TypeOfTimeIntervals::SameStartTimeForecastIncremented;
        case 3:
            return TypeOfTimeIntervals::StartIncrementedForecastDecrementedConstantValid;
        case 4:
            return TypeOfTimeIntervals::StartDecrementedForecastIncrementedConstantValid;
        case 5:
            return TypeOfTimeIntervals::FloatingSubinterval;
        case 255:
            return TypeOfTimeIntervals::Missing;
        default:
            throw Mars2GribTableException("Invalid TypeOfTimeIntervals numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..5,255}",
                                          Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
template <class Cntx_t>
inline std::string enum2name_TypeOfTimeIntervals_or_throw(TypeOfTimeIntervals value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfTimeIntervals::Reserved:
            {
                std::string result = "reserved";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfTimeIntervals::SameForecastTimeStartIncremented:
            {
                std::string result = "same-forecast-time-start-incremented";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfTimeIntervals::SameStartTimeForecastIncremented:
            {
                std::string result = "same-start-time-forecast-incremented";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfTimeIntervals::StartIncrementedForecastDecrementedConstantValid:
            {
                std::string result = "start-incremented-forecast-decremented-constant-valid";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfTimeIntervals::StartDecrementedForecastIncrementedConstantValid:
            {
                std::string result = "start-decremented-forecast-incremented-constant-valid";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfTimeIntervals::FloatingSubinterval:
            {
                std::string result = "floating-subinterval";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfTimeIntervals::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
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
template <class Cntx_t>
inline TypeOfTimeIntervals name2enum_TypeOfTimeIntervals_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "reserved") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::Reserved;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "same-forecast-time-start-incremented") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::SameForecastTimeStartIncremented;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "same-start-time-forecast-incremented") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::SameStartTimeForecastIncremented;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "start-incremented-forecast-decremented-constant-valid") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::StartIncrementedForecastDecrementedConstantValid;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "start-decremented-forecast-incremented-constant-valid") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::StartDecrementedForecastIncrementedConstantValid;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "floating-subinterval") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::FloatingSubinterval;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "missing") {
        {
            TypeOfTimeIntervals result = TypeOfTimeIntervals::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
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
template <class Cntx_t>
inline TypeOfTimeIntervals long2enum_TypeOfTimeIntervals_or_throw(long value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::Reserved;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 1:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::SameForecastTimeStartIncremented;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 2:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::SameStartTimeForecastIncremented;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 3:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::StartIncrementedForecastDecrementedConstantValid;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 4:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::StartDecrementedForecastIncrementedConstantValid;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 5:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::FloatingSubinterval;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 255:
            {
                TypeOfTimeIntervals result = TypeOfTimeIntervals::Missing;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException("Invalid TypeOfTimeIntervals numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..5,255}",
                                          Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

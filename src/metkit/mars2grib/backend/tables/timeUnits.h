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
/// @brief GRIB time units (Code Table 4.4).
///
/// This enumeration represents the GRIB code values associated with
/// time units as defined in GRIB2 Code Table 4.4.
///
/// The numeric values map **directly** to the official GRIB code table
/// and must not be changed manually.
///
/// @section Source of truth
/// GRIB2 Code Table 4.4:
/// Units of time range
///
/// @note
/// This enum is a pure GRIB table representation.
/// No semantic interpretation or policy decisions are encoded here.
///
/// @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: critical]
/// - Generate this enum automatically from ecCodes GRIB tables
/// to guarantee alignment with the runtime ecCodes version.
///
enum class TimeUnit : long {
    Minute  = 0,
    Hour    = 1,
    Day     = 2,
    Month   = 3,
    Year    = 4,
    Decade  = 5,  // 10 years
    Normal  = 6,  // 30 years
    Century = 7,  // 100 years

    Hours3  = 10,
    Hours6  = 11,
    Hours12 = 12,

    Second = 13,

    Missing = 255
};

///
/// @brief Convert a symbolic time-unit name to a GRIB `TimeUnit`.
///
/// Performs a strict mapping from a string identifier to the corresponding
/// GRIB time unit code.
///
/// Supported names:
/// - "minute"
/// - "hour"
/// - "day"
/// - "month"
/// - "year"
/// - "decade"
/// - "normal"
/// - "century"
/// - "3h"
/// - "6h"
/// - "12h"
/// - "second"
/// - "missing"
///
/// @param[in] name Symbolic name of the time unit
///
/// @return Corresponding `TimeUnit` enumeration value
///
/// @throws Mars2GribTableException
/// If the name is not a supported GRIB time unit
///
/// @note
/// - Mapping is case-sensitive by design.
/// - No normalization or aliasing is performed.
///
template <class Cntx_t>
inline TimeUnit name2enum_TimeUnit_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "minute") {
        {
            TimeUnit result = TimeUnit::Minute;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "hour") {
        {
            TimeUnit result = TimeUnit::Hour;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "day") {
        {
            TimeUnit result = TimeUnit::Day;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "month") {
        {
            TimeUnit result = TimeUnit::Month;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "year") {
        {
            TimeUnit result = TimeUnit::Year;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "decade") {
        {
            TimeUnit result = TimeUnit::Decade;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "normal") {
        {
            TimeUnit result = TimeUnit::Normal;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "century") {
        {
            TimeUnit result = TimeUnit::Century;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    if (name == "3h") {
        {
            TimeUnit result = TimeUnit::Hours3;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "6h") {
        {
            TimeUnit result = TimeUnit::Hours6;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "12h") {
        {
            TimeUnit result = TimeUnit::Hours12;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    if (name == "second") {
        {
            TimeUnit result = TimeUnit::Second;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "missing") {
        {
            TimeUnit result = TimeUnit::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    std::string err = "Invalid TimeUnit name: actual='" + name +
                      "', expected={minute,hour,day,month,year,decade,normal,century,"
                      "3h,6h,12h,second,missing}";
    throw Mars2GribTableException(err, Here());

    mars2gribUnreachable();
}

///
/// @brief Convert a GRIB `TimeUnit` enumeration to its symbolic name.
///
/// Performs a strict mapping from a GRIB time unit code to its
/// canonical string representation.
///
/// @param[in] value GRIB `TimeUnit` enumeration value
///
/// @return Canonical symbolic name of the time unit
///
/// @throws Mars2GribTableException
/// If the enum value is not supported
///
/// @note
/// - Returned strings are stable and suitable for logging, YAML,
/// diagnostics, and round-tripping via `name2enum_TimeUnit_or_throw`.
///
template <class Cntx_t>
inline std::string enum2name_TimeUnit_or_throw(TimeUnit value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TimeUnit::Minute:
            {
                std::string result = "minute";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Hour:
            {
                std::string result = "hour";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Day:
            {
                std::string result = "day";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Month:
            {
                std::string result = "month";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Year:
            {
                std::string result = "year";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Decade:
            {
                std::string result = "decade";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Normal:
            {
                std::string result = "normal";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Century:
            {
                std::string result = "century";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }

        case TimeUnit::Hours3:
            {
                std::string result = "3h";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Hours6:
            {
                std::string result = "6h";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Hours12:
            {
                std::string result = "12h";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }

        case TimeUnit::Second:
            {
                std::string result = "second";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TimeUnit::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
    }

    std::string err = "Invalid TimeUnit enum value: actual='" + std::to_string(static_cast<long>(value)) + "'";
    throw Mars2GribTableException(err, Here());

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::tables
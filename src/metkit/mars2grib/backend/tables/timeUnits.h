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
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::tables {

/**
 * @brief GRIB time units (Code Table 4.4).
 *
 * This enumeration represents the GRIB code values associated with
 * time units as defined in GRIB2 Code Table 4.4.
 *
 * The numeric values map **directly** to the official GRIB code table
 * and must not be changed manually.
 *
 * @section Source of truth
 * GRIB2 Code Table 4.4:
 *   Units of time range
 *
 * @note
 * This enum is a pure GRIB table representation.
 * No semantic interpretation or policy decisions are encoded here.
 *
 * @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: critical]
 * - Generate this enum automatically from ecCodes GRIB tables
 *   to guarantee alignment with the runtime ecCodes version.
 */
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

/**
 * @brief Convert a symbolic time-unit name to a GRIB `TimeUnit`.
 *
 * Performs a strict mapping from a string identifier to the corresponding
 * GRIB time unit code.
 *
 * Supported names:
 *  - "minute"
 *  - "hour"
 *  - "day"
 *  - "month"
 *  - "year"
 *  - "decade"
 *  - "normal"
 *  - "century"
 *  - "3h"
 *  - "6h"
 *  - "12h"
 *  - "second"
 *  - "missing"
 *
 * @param[in] name Symbolic name of the time unit
 *
 * @return Corresponding `TimeUnit` enumeration value
 *
 * @throws Mars2GribTableException
 *         If the name is not a supported GRIB time unit
 *
 * @note
 * - Mapping is case-sensitive by design.
 * - No normalization or aliasing is performed.
 */
inline TimeUnit name2enum_TimeUnit_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "minute")
        return TimeUnit::Minute;
    if (name == "hour")
        return TimeUnit::Hour;
    if (name == "day")
        return TimeUnit::Day;
    if (name == "month")
        return TimeUnit::Month;
    if (name == "year")
        return TimeUnit::Year;
    if (name == "decade")
        return TimeUnit::Decade;
    if (name == "normal")
        return TimeUnit::Normal;
    if (name == "century")
        return TimeUnit::Century;

    if (name == "3h")
        return TimeUnit::Hours3;
    if (name == "6h")
        return TimeUnit::Hours6;
    if (name == "12h")
        return TimeUnit::Hours12;

    if (name == "second")
        return TimeUnit::Second;
    if (name == "missing")
        return TimeUnit::Missing;

    std::string err = "Invalid TimeUnit name: actual='" + name +
                      "', expected={minute,hour,day,month,year,decade,normal,century,"
                      "3h,6h,12h,second,missing}";
    throw Mars2GribTableException(err, Here());

    __builtin_unreachable();
}

/**
 * @brief Convert a GRIB `TimeUnit` enumeration to its symbolic name.
 *
 * Performs a strict mapping from a GRIB time unit code to its
 * canonical string representation.
 *
 * @param[in] value GRIB `TimeUnit` enumeration value
 *
 * @return Canonical symbolic name of the time unit
 *
 * @throws Mars2GribTableException
 *         If the enum value is not supported
 *
 * @note
 * - Returned strings are stable and suitable for logging, YAML,
 *   diagnostics, and round-tripping via `name2enum_TimeUnit_or_throw`.
 */
inline std::string enum2name_TimeUnit_or_throw(TimeUnit value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TimeUnit::Minute:
            return "minute";
        case TimeUnit::Hour:
            return "hour";
        case TimeUnit::Day:
            return "day";
        case TimeUnit::Month:
            return "month";
        case TimeUnit::Year:
            return "year";
        case TimeUnit::Decade:
            return "decade";
        case TimeUnit::Normal:
            return "normal";
        case TimeUnit::Century:
            return "century";

        case TimeUnit::Hours3:
            return "3h";
        case TimeUnit::Hours6:
            return "6h";
        case TimeUnit::Hours12:
            return "12h";

        case TimeUnit::Second:
            return "second";
        case TimeUnit::Missing:
            return "missing";
    }

    std::string err = "Invalid TimeUnit enum value: actual='" + std::to_string(static_cast<long>(value)) + "'";
    throw Mars2GribTableException(err, Here());

    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::tables
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
/// @brief Type of interval.
///
/// This enumeration represents GRIB code values defining how an interval
/// is interpreted with respect to its first and second limits.
///
/// The numeric values map **directly** to ecCodes GRIB table 4.91
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
enum class TypeOfInterval : long {
    SmallerThanFirstLimit                = 0,
    GreaterThanSecondLimit               = 1,
    BetweenFirstInclusiveSecondExclusive = 2,
    GreaterThanFirstLimit                = 3,
    SmallerThanSecondLimit               = 4,
    SmallerOrEqualFirstLimit             = 5,
    GreaterOrEqualSecondLimit            = 6,
    BetweenFirstInclusiveSecondInclusive = 7,
    GreaterOrEqualFirstLimit             = 8,
    SmallerOrEqualSecondLimit            = 9,
    BetweenFirstExclusiveSecondInclusive = 10,
    EqualFirstLimit                      = 11,
    Missing                              = 255
};

///
/// @brief Convert `TypeOfInterval` to its canonical name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string name
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is invalid.
///
inline std::string enum2name_TypeOfInterval_or_throw(TypeOfInterval value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfInterval::SmallerThanFirstLimit:
            return "smaller-than-first-limit";
        case TypeOfInterval::GreaterThanSecondLimit:
            return "greater-than-second-limit";
        case TypeOfInterval::BetweenFirstInclusiveSecondExclusive:
            return "between-first-inclusive-second-exclusive";
        case TypeOfInterval::GreaterThanFirstLimit:
            return "greater-than-first-limit";
        case TypeOfInterval::SmallerThanSecondLimit:
            return "smaller-than-second-limit";
        case TypeOfInterval::SmallerOrEqualFirstLimit:
            return "smaller-or-equal-first-limit";
        case TypeOfInterval::GreaterOrEqualSecondLimit:
            return "greater-or-equal-second-limit";
        case TypeOfInterval::BetweenFirstInclusiveSecondInclusive:
            return "between-first-inclusive-second-inclusive";
        case TypeOfInterval::GreaterOrEqualFirstLimit:
            return "greater-or-equal-first-limit";
        case TypeOfInterval::SmallerOrEqualSecondLimit:
            return "smaller-or-equal-second-limit";
        case TypeOfInterval::BetweenFirstExclusiveSecondInclusive:
            return "between-first-exclusive-second-inclusive";
        case TypeOfInterval::EqualFirstLimit:
            return "equal-first-limit";
        case TypeOfInterval::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid TypeOfInterval enum value", Here());
    }
}


///
/// @brief Convert a canonical name to `TypeOfInterval`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the name is not recognized.
///
inline TypeOfInterval name2enum_TypeOfInterval_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "smaller-than-first-limit")
        return TypeOfInterval::SmallerThanFirstLimit;
    else if (name == "greater-than-second-limit")
        return TypeOfInterval::GreaterThanSecondLimit;
    else if (name == "between-first-inclusive-second-exclusive")
        return TypeOfInterval::BetweenFirstInclusiveSecondExclusive;
    else if (name == "greater-than-first-limit")
        return TypeOfInterval::GreaterThanFirstLimit;
    else if (name == "smaller-than-second-limit")
        return TypeOfInterval::SmallerThanSecondLimit;
    else if (name == "smaller-or-equal-first-limit")
        return TypeOfInterval::SmallerOrEqualFirstLimit;
    else if (name == "greater-or-equal-second-limit")
        return TypeOfInterval::GreaterOrEqualSecondLimit;
    else if (name == "between-first-inclusive-second-inclusive")
        return TypeOfInterval::BetweenFirstInclusiveSecondInclusive;
    else if (name == "greater-or-equal-first-limit")
        return TypeOfInterval::GreaterOrEqualFirstLimit;
    else if (name == "smaller-or-equal-second-limit")
        return TypeOfInterval::SmallerOrEqualSecondLimit;
    else if (name == "between-first-exclusive-second-inclusive")
        return TypeOfInterval::BetweenFirstExclusiveSecondInclusive;
    else if (name == "equal-first-limit")
        return TypeOfInterval::EqualFirstLimit;
    else if (name == "missing")
        return TypeOfInterval::Missing;
    else
        throw Mars2GribTableException("Invalid TypeOfInterval name: '" + name + "'", Here());
}


///
/// @brief Convert a numeric GRIB code to `TypeOfInterval`.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the value is not defined in the GRIB table.
///
inline TypeOfInterval long2enum_TypeOfInterval_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfInterval::SmallerThanFirstLimit;
        case 1:
            return TypeOfInterval::GreaterThanSecondLimit;
        case 2:
            return TypeOfInterval::BetweenFirstInclusiveSecondExclusive;
        case 3:
            return TypeOfInterval::GreaterThanFirstLimit;
        case 4:
            return TypeOfInterval::SmallerThanSecondLimit;
        case 5:
            return TypeOfInterval::SmallerOrEqualFirstLimit;
        case 6:
            return TypeOfInterval::GreaterOrEqualSecondLimit;
        case 7:
            return TypeOfInterval::BetweenFirstInclusiveSecondInclusive;
        case 8:
            return TypeOfInterval::GreaterOrEqualFirstLimit;
        case 9:
            return TypeOfInterval::SmallerOrEqualSecondLimit;
        case 10:
            return TypeOfInterval::BetweenFirstExclusiveSecondInclusive;
        case 11:
            return TypeOfInterval::EqualFirstLimit;
        case 255:
            return TypeOfInterval::Missing;
        default:
            throw Mars2GribTableException(
                "Invalid TypeOfInterval numeric value: actual='" + std::to_string(value) + "', expected={0..11,255}",
                Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

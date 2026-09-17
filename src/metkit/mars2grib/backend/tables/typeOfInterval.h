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
template <class Cntx_t>
inline std::string enum2name_TypeOfInterval_or_throw(TypeOfInterval value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfInterval::SmallerThanFirstLimit:
            {
                std::string result = "smaller-than-first-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::GreaterThanSecondLimit:
            {
                std::string result = "greater-than-second-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::BetweenFirstInclusiveSecondExclusive:
            {
                std::string result = "between-first-inclusive-second-exclusive";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::GreaterThanFirstLimit:
            {
                std::string result = "greater-than-first-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::SmallerThanSecondLimit:
            {
                std::string result = "smaller-than-second-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::SmallerOrEqualFirstLimit:
            {
                std::string result = "smaller-or-equal-first-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::GreaterOrEqualSecondLimit:
            {
                std::string result = "greater-or-equal-second-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::BetweenFirstInclusiveSecondInclusive:
            {
                std::string result = "between-first-inclusive-second-inclusive";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::GreaterOrEqualFirstLimit:
            {
                std::string result = "greater-or-equal-first-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::SmallerOrEqualSecondLimit:
            {
                std::string result = "smaller-or-equal-second-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::BetweenFirstExclusiveSecondInclusive:
            {
                std::string result = "between-first-exclusive-second-inclusive";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::EqualFirstLimit:
            {
                std::string result = "equal-first-limit";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfInterval::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
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
template <class Cntx_t>
inline TypeOfInterval name2enum_TypeOfInterval_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "smaller-than-first-limit") {
        {
            TypeOfInterval result = TypeOfInterval::SmallerThanFirstLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "greater-than-second-limit") {
        {
            TypeOfInterval result = TypeOfInterval::GreaterThanSecondLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "between-first-inclusive-second-exclusive") {
        {
            TypeOfInterval result = TypeOfInterval::BetweenFirstInclusiveSecondExclusive;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "greater-than-first-limit") {
        {
            TypeOfInterval result = TypeOfInterval::GreaterThanFirstLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "smaller-than-second-limit") {
        {
            TypeOfInterval result = TypeOfInterval::SmallerThanSecondLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "smaller-or-equal-first-limit") {
        {
            TypeOfInterval result = TypeOfInterval::SmallerOrEqualFirstLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "greater-or-equal-second-limit") {
        {
            TypeOfInterval result = TypeOfInterval::GreaterOrEqualSecondLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "between-first-inclusive-second-inclusive") {
        {
            TypeOfInterval result = TypeOfInterval::BetweenFirstInclusiveSecondInclusive;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "greater-or-equal-first-limit") {
        {
            TypeOfInterval result = TypeOfInterval::GreaterOrEqualFirstLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "smaller-or-equal-second-limit") {
        {
            TypeOfInterval result = TypeOfInterval::SmallerOrEqualSecondLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "between-first-exclusive-second-inclusive") {
        {
            TypeOfInterval result = TypeOfInterval::BetweenFirstExclusiveSecondInclusive;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "equal-first-limit") {
        {
            TypeOfInterval result = TypeOfInterval::EqualFirstLimit;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "missing") {
        {
            TypeOfInterval result = TypeOfInterval::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
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
template <class Cntx_t>
inline TypeOfInterval long2enum_TypeOfInterval_or_throw(long value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            {
                TypeOfInterval result = TypeOfInterval::SmallerThanFirstLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 1:
            {
                TypeOfInterval result = TypeOfInterval::GreaterThanSecondLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 2:
            {
                TypeOfInterval result = TypeOfInterval::BetweenFirstInclusiveSecondExclusive;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 3:
            {
                TypeOfInterval result = TypeOfInterval::GreaterThanFirstLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 4:
            {
                TypeOfInterval result = TypeOfInterval::SmallerThanSecondLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 5:
            {
                TypeOfInterval result = TypeOfInterval::SmallerOrEqualFirstLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 6:
            {
                TypeOfInterval result = TypeOfInterval::GreaterOrEqualSecondLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 7:
            {
                TypeOfInterval result = TypeOfInterval::BetweenFirstInclusiveSecondInclusive;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 8:
            {
                TypeOfInterval result = TypeOfInterval::GreaterOrEqualFirstLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 9:
            {
                TypeOfInterval result = TypeOfInterval::SmallerOrEqualSecondLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 10:
            {
                TypeOfInterval result = TypeOfInterval::BetweenFirstExclusiveSecondInclusive;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 11:
            {
                TypeOfInterval result = TypeOfInterval::EqualFirstLimit;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 255:
            {
                TypeOfInterval result = TypeOfInterval::Missing;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException(
                "Invalid TypeOfInterval numeric value: actual='" + std::to_string(value) + "', expected={0..11,255}",
                Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Probability Type.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.9 – Probability type**.
///
/// The numeric values of the enumerators map **directly** to the GRIB
/// code table and must not be modified manually.
///
/// @note
/// The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-level representation** only.
/// No policy, deduction, or defaulting logic must be embedded here.
///
/// @section Source of truth
/// GRIB2 Code Table 4.9 – Probability type, as implemented in ecCodes.
///
enum class ProbabilityType : long {
    BelowLowerLimit            = 0,
    AboveUpperLimit            = 1,
    BetweenLowerAndUpperLimits = 2,
    AboveLowerLimit            = 3,
    BelowUpperLimit            = 4,
    EqualToLowerLimit          = 5,
    AboveNormalCategory        = 6,
    NearNormalCategory         = 7,
    BelowNormalCategory        = 8,
    CountsOfCategoricalBoolean = 9,
    Quantile                   = 10,
    Missing                    = 255
};


///
/// @brief Convert `ProbabilityType` to its canonical string name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
inline std::string enum2name_ProbabilityType_or_throw(ProbabilityType value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case ProbabilityType::BelowLowerLimit:
            return "below-lower-limit";
        case ProbabilityType::AboveUpperLimit:
            return "above-upper-limit";
        case ProbabilityType::BetweenLowerAndUpperLimits:
            return "between-lower-and-upper-limits";
        case ProbabilityType::AboveLowerLimit:
            return "above-lower-limit";
        case ProbabilityType::BelowUpperLimit:
            return "below-upper-limit";
        case ProbabilityType::EqualToLowerLimit:
            return "equal-to-lower-limit";
        case ProbabilityType::AboveNormalCategory:
            return "above-normal-category";
        case ProbabilityType::NearNormalCategory:
            return "near-normal-category";
        case ProbabilityType::BelowNormalCategory:
            return "below-normal-category";
        case ProbabilityType::CountsOfCategoricalBoolean:
            return "counts-of-categorical-boolean";
        case ProbabilityType::Quantile:
            return "quantile";
        case ProbabilityType::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid ProbabilityType enum value", Here());
    }
}


///
/// @brief Convert a canonical string name to `ProbabilityType`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `ProbabilityType` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
inline ProbabilityType name2enum_ProbabilityType_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "below-lower-limit") {
        return ProbabilityType::BelowLowerLimit;
    }
    else if (name == "above-upper-limit") {
        return ProbabilityType::AboveUpperLimit;
    }
    else if (name == "between-lower-and-upper-limits") {
        return ProbabilityType::BetweenLowerAndUpperLimits;
    }
    else if (name == "above-lower-limit") {
        return ProbabilityType::AboveLowerLimit;
    }
    else if (name == "below-upper-limit") {
        return ProbabilityType::BelowUpperLimit;
    }
    else if (name == "equal-to-lower-limit") {
        return ProbabilityType::EqualToLowerLimit;
    }
    else if (name == "above-normal-category") {
        return ProbabilityType::AboveNormalCategory;
    }
    else if (name == "near-normal-category") {
        return ProbabilityType::NearNormalCategory;
    }
    else if (name == "below-normal-category") {
        return ProbabilityType::BelowNormalCategory;
    }
    else if (name == "counts-of-categorical-boolean") {
        return ProbabilityType::CountsOfCategoricalBoolean;
    }
    else if (name == "quantile") {
        return ProbabilityType::Quantile;
    }
    else if (name == "missing") {
        return ProbabilityType::Missing;
    }
    else {
        throw Mars2GribTableException("Invalid ProbabilityType name: '" + name + "'", Here());
    }
}


///
/// @brief Convert a numeric GRIB code to `ProbabilityType`.
///
/// Only officially defined GRIB values are accepted.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `ProbabilityType` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a valid
/// GRIB Code Table 4.9 entry.
///
inline ProbabilityType long2enum_ProbabilityType_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return ProbabilityType::BelowLowerLimit;
        case 1:
            return ProbabilityType::AboveUpperLimit;
        case 2:
            return ProbabilityType::BetweenLowerAndUpperLimits;
        case 3:
            return ProbabilityType::AboveLowerLimit;
        case 4:
            return ProbabilityType::BelowUpperLimit;
        case 5:
            return ProbabilityType::EqualToLowerLimit;
        case 6:
            return ProbabilityType::AboveNormalCategory;
        case 7:
            return ProbabilityType::NearNormalCategory;
        case 8:
            return ProbabilityType::BelowNormalCategory;
        case 9:
            return ProbabilityType::CountsOfCategoricalBoolean;
        case 10:
            return ProbabilityType::Quantile;
        case 255:
            return ProbabilityType::Missing;
        default:
            throw Mars2GribTableException("Invalid ProbabilityType value: " + std::to_string(value), Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

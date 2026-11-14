// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Spatial Vicinity Type.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.103 – Spatial vicinity type**.
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
/// GRIB2 Code Table 4.103 – Spatial vicinity type, as implemented in ecCodes.
///
enum class SpatialVicinityType : long {
    Circle      = 0,
    Rectangle   = 1,
    Square      = 2,
    Wedge       = 3,
    GridBoxSpan = 4,
    Missing     = 255
};


///
/// @brief Convert `SpatialVicinityType` to its canonical string name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
inline std::string enum2name_SpatialVicinityType_or_throw(SpatialVicinityType value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case SpatialVicinityType::Circle:
            return "circle";
        case SpatialVicinityType::Rectangle:
            return "rectangle";
        case SpatialVicinityType::Square:
            return "square";
        case SpatialVicinityType::Wedge:
            return "wedge";
        case SpatialVicinityType::GridBoxSpan:
            return "grid-box-span";
        case SpatialVicinityType::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid SpatialVicinityType enum value", Here());
    }
}


///
/// @brief Convert a canonical string name to `SpatialVicinityType`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `SpatialVicinityType` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
inline SpatialVicinityType name2enum_SpatialVicinityType_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "circle") {
        return SpatialVicinityType::Circle;
    }
    else if (name == "rectangle") {
        return SpatialVicinityType::Rectangle;
    }
    else if (name == "square") {
        return SpatialVicinityType::Square;
    }
    else if (name == "wedge") {
        return SpatialVicinityType::Wedge;
    }
    else if (name == "grid-box-span") {
        return SpatialVicinityType::GridBoxSpan;
    }
    else if (name == "missing") {
        return SpatialVicinityType::Missing;
    }
    else {
        throw Mars2GribTableException("Invalid SpatialVicinityType name: '" + name + "'", Here());
    }
}


///
/// @brief Convert a numeric GRIB code to `SpatialVicinityType`.
///
/// Only officially defined GRIB values are accepted.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `SpatialVicinityType` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a valid
/// GRIB Code Table 4.103 entry.
///
inline SpatialVicinityType long2enum_SpatialVicinityType_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return SpatialVicinityType::Circle;
        case 1:
            return SpatialVicinityType::Rectangle;
        case 2:
            return SpatialVicinityType::Square;
        case 3:
            return SpatialVicinityType::Wedge;
        case 4:
            return SpatialVicinityType::GridBoxSpan;
        case 255:
            return SpatialVicinityType::Missing;
        default:
            throw Mars2GribTableException("Invalid SpatialVicinityType value: " + std::to_string(value), Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

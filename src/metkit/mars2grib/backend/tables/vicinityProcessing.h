// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Vicinity Processing.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.104 – Spatial and temporal vicinity processing**.
///
/// The same code table is used for both `spatialVicinityProcessing` and
/// `temporalVicinityProcessing`.
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
/// GRIB2 Code Table 4.104 – Spatial and temporal vicinity processing, as
/// implemented in ecCodes.
///
enum class VicinityProcessing : long {
    Average           = 0,
    Maximum           = 2,
    Minimum           = 3,
    Range             = 4,
    StandardDeviation = 6,
    Sum               = 11,
    Quantile          = 190,
    Categorical       = 191,
    Missing           = 255
};


///
/// @brief Convert `VicinityProcessing` to its canonical string name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
inline std::string enum2name_VicinityProcessing_or_throw(VicinityProcessing value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case VicinityProcessing::Average:
            return "average";
        case VicinityProcessing::Maximum:
            return "maximum";
        case VicinityProcessing::Minimum:
            return "minimum";
        case VicinityProcessing::Range:
            return "range";
        case VicinityProcessing::StandardDeviation:
            return "standard-deviation";
        case VicinityProcessing::Sum:
            return "sum";
        case VicinityProcessing::Quantile:
            return "quantile";
        case VicinityProcessing::Categorical:
            return "categorical";
        case VicinityProcessing::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid VicinityProcessing enum value", Here());
    }
}


///
/// @brief Convert a canonical string name to `VicinityProcessing`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `VicinityProcessing` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
inline VicinityProcessing name2enum_VicinityProcessing_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "average") {
        return VicinityProcessing::Average;
    }
    else if (name == "maximum") {
        return VicinityProcessing::Maximum;
    }
    else if (name == "minimum") {
        return VicinityProcessing::Minimum;
    }
    else if (name == "range") {
        return VicinityProcessing::Range;
    }
    else if (name == "standard-deviation") {
        return VicinityProcessing::StandardDeviation;
    }
    else if (name == "sum") {
        return VicinityProcessing::Sum;
    }
    else if (name == "quantile") {
        return VicinityProcessing::Quantile;
    }
    else if (name == "categorical") {
        return VicinityProcessing::Categorical;
    }
    else if (name == "missing") {
        return VicinityProcessing::Missing;
    }
    else {
        throw Mars2GribTableException("Invalid VicinityProcessing name: '" + name + "'", Here());
    }
}


///
/// @brief Convert a numeric GRIB code to `VicinityProcessing`.
///
/// Only officially defined GRIB values are accepted.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `VicinityProcessing` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a valid
/// GRIB Code Table 4.104 entry.
///
inline VicinityProcessing long2enum_VicinityProcessing_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return VicinityProcessing::Average;
        case 2:
            return VicinityProcessing::Maximum;
        case 3:
            return VicinityProcessing::Minimum;
        case 4:
            return VicinityProcessing::Range;
        case 6:
            return VicinityProcessing::StandardDeviation;
        case 11:
            return VicinityProcessing::Sum;
        case 190:
            return VicinityProcessing::Quantile;
        case 191:
            return VicinityProcessing::Categorical;
        case 255:
            return VicinityProcessing::Missing;
        default:
            throw Mars2GribTableException("Invalid VicinityProcessing value: " + std::to_string(value), Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables

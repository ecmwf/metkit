// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Type of Reference Dataset.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.100 – Type of reference dataset**.
///
/// The values describe the kind of dataset used as reference for the
/// product (e.g. the reforecast dataset used to build a model climate).
///
/// The numeric values of the enumerators map **directly** to the GRIB
/// code table and must not be modified manually.
///
/// @note
/// - Values `6-191` are reserved and `192-254` are reserved for local use:
///   they are not supported.
/// - The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-level representation** only.
/// No policy, deduction, or defaulting logic must be embedded here.
///
/// @section Source of truth
/// GRIB2 Code Table 4.100 – Type of reference dataset, as implemented in ecCodes.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and its conversion helpers automatically from
/// ecCodes GRIB code tables at build or configure time to avoid drift.
///
enum class TypeOfReferenceDataset : long {
    Analysis            = 0,
    Forecast            = 1,
    Reforecast          = 2,
    Reanalysis          = 3,
    ClimateProjection   = 4,
    GriddedObservations = 5,
    Missing             = 255
};

///
/// @brief Convert `TypeOfReferenceDataset` to its canonical string name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
inline std::string enum2name_TypeOfReferenceDataset_or_throw(TypeOfReferenceDataset value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfReferenceDataset::Analysis:
            return "analysis";
        case TypeOfReferenceDataset::Forecast:
            return "forecast";
        case TypeOfReferenceDataset::Reforecast:
            return "reforecast";
        case TypeOfReferenceDataset::Reanalysis:
            return "reanalysis";
        case TypeOfReferenceDataset::ClimateProjection:
            return "climate-projection";
        case TypeOfReferenceDataset::GriddedObservations:
            return "gridded-observations";
        case TypeOfReferenceDataset::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid TypeOfReferenceDataset enum value", Here());
    }

    mars2gribUnreachable();
}

///
/// @brief Convert a canonical string name to `TypeOfReferenceDataset`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `TypeOfReferenceDataset` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
inline TypeOfReferenceDataset name2enum_TypeOfReferenceDataset_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "analysis") {
        return TypeOfReferenceDataset::Analysis;
    }
    if (name == "forecast") {
        return TypeOfReferenceDataset::Forecast;
    }
    if (name == "reforecast") {
        return TypeOfReferenceDataset::Reforecast;
    }
    if (name == "reanalysis") {
        return TypeOfReferenceDataset::Reanalysis;
    }
    if (name == "climate-projection") {
        return TypeOfReferenceDataset::ClimateProjection;
    }
    if (name == "gridded-observations") {
        return TypeOfReferenceDataset::GriddedObservations;
    }
    if (name == "missing") {
        return TypeOfReferenceDataset::Missing;
    }

    throw Mars2GribTableException("Invalid TypeOfReferenceDataset name: '" + name + "'", Here());

    mars2gribUnreachable();
}

///
/// @brief Convert a numeric GRIB code to `TypeOfReferenceDataset`.
///
/// Only values defined in **GRIB2 Code Table 4.100** are accepted:
/// `{0..5, 255}`. Reserved and local-use values are rejected.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `TypeOfReferenceDataset` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a supported entry.
///
/// @important
/// This function performs **pure table validation**.
/// Any semantic interpretation must be handled by the calling deduction.
///
inline TypeOfReferenceDataset long2enum_TypeOfReferenceDataset_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfReferenceDataset::Analysis;
        case 1:
            return TypeOfReferenceDataset::Forecast;
        case 2:
            return TypeOfReferenceDataset::Reforecast;
        case 3:
            return TypeOfReferenceDataset::Reanalysis;
        case 4:
            return TypeOfReferenceDataset::ClimateProjection;
        case 5:
            return TypeOfReferenceDataset::GriddedObservations;
        case 255:
            return TypeOfReferenceDataset::Missing;
        default:
            throw Mars2GribTableException("Invalid TypeOfReferenceDataset numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..5,255}",
                                          Here());
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::tables

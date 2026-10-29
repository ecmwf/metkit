// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Type of Relationship to Reference Dataset.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.101 – Type of relationship to reference dataset**
/// (GRIB key `typeOfRelationToReferenceDataset`).
///
/// The values describe how the product relates to its reference dataset
/// (anomaly, EFI, SOT, ...).
///
/// The numeric values of the enumerators map **directly** to the GRIB
/// code table and must not be modified manually.
///
/// @note
/// - Values `4-19` and `25-191` are reserved and `192-254` are reserved for
///   local use: they are not supported.
/// - The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-level representation** only.
/// No policy, deduction, or defaulting logic must be embedded here.
///
/// @section Source of truth
/// GRIB2 Code Table 4.101 – Type of relationship to reference dataset,
/// as implemented in ecCodes.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and its conversion helpers automatically from
/// ecCodes GRIB code tables at build or configure time to avoid drift.
///
enum class TypeOfRelationToReferenceDataset : long {
    Anomaly                  = 0,
    StandardizedAnomaly      = 1,
    Significance             = 2,  // Wilcoxon-Mann-Whitney
    Climatology              = 3,
    ExtremeForecastIndex     = 20,  // EFI
    ShiftOfTails             = 21,  // SOT
    AnomalyOfProbabilities   = 22,
    StandardizedDroughtIndex = 23,
    CrossingPointForecast    = 24,  // CPF
    Missing                  = 255
};

///
/// @brief Convert `TypeOfRelationToReferenceDataset` to its canonical string name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
inline std::string enum2name_TypeOfRelationToReferenceDataset_or_throw(TypeOfRelationToReferenceDataset value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfRelationToReferenceDataset::Anomaly:
            return "anomaly";
        case TypeOfRelationToReferenceDataset::StandardizedAnomaly:
            return "standardized-anomaly";
        case TypeOfRelationToReferenceDataset::Significance:
            return "significance";
        case TypeOfRelationToReferenceDataset::Climatology:
            return "climatology";
        case TypeOfRelationToReferenceDataset::ExtremeForecastIndex:
            return "efi";
        case TypeOfRelationToReferenceDataset::ShiftOfTails:
            return "sot";
        case TypeOfRelationToReferenceDataset::AnomalyOfProbabilities:
            return "anomaly-of-probabilities";
        case TypeOfRelationToReferenceDataset::StandardizedDroughtIndex:
            return "standardized-drought-index";
        case TypeOfRelationToReferenceDataset::CrossingPointForecast:
            return "cpf";
        case TypeOfRelationToReferenceDataset::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid TypeOfRelationToReferenceDataset enum value", Here());
    }

    mars2gribUnreachable();
}

///
/// @brief Convert a canonical string name to `TypeOfRelationToReferenceDataset`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `TypeOfRelationToReferenceDataset` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
inline TypeOfRelationToReferenceDataset name2enum_TypeOfRelationToReferenceDataset_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "anomaly") {
        return TypeOfRelationToReferenceDataset::Anomaly;
    }
    if (name == "standardized-anomaly") {
        return TypeOfRelationToReferenceDataset::StandardizedAnomaly;
    }
    if (name == "significance") {
        return TypeOfRelationToReferenceDataset::Significance;
    }
    if (name == "climatology") {
        return TypeOfRelationToReferenceDataset::Climatology;
    }
    if (name == "efi") {
        return TypeOfRelationToReferenceDataset::ExtremeForecastIndex;
    }
    if (name == "sot") {
        return TypeOfRelationToReferenceDataset::ShiftOfTails;
    }
    if (name == "anomaly-of-probabilities") {
        return TypeOfRelationToReferenceDataset::AnomalyOfProbabilities;
    }
    if (name == "standardized-drought-index") {
        return TypeOfRelationToReferenceDataset::StandardizedDroughtIndex;
    }
    if (name == "cpf") {
        return TypeOfRelationToReferenceDataset::CrossingPointForecast;
    }
    if (name == "missing") {
        return TypeOfRelationToReferenceDataset::Missing;
    }

    throw Mars2GribTableException("Invalid TypeOfRelationToReferenceDataset name: '" + name + "'", Here());

    mars2gribUnreachable();
}

///
/// @brief Convert a numeric GRIB code to `TypeOfRelationToReferenceDataset`.
///
/// Only values defined in **GRIB2 Code Table 4.101** are accepted:
/// `{0..3, 20..24, 255}`. Reserved and local-use values are rejected.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `TypeOfRelationToReferenceDataset` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a supported entry.
///
/// @important
/// This function performs **pure table validation**.
/// Any semantic interpretation must be handled by the calling deduction.
///
inline TypeOfRelationToReferenceDataset long2enum_TypeOfRelationToReferenceDataset_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfRelationToReferenceDataset::Anomaly;
        case 1:
            return TypeOfRelationToReferenceDataset::StandardizedAnomaly;
        case 2:
            return TypeOfRelationToReferenceDataset::Significance;
        case 3:
            return TypeOfRelationToReferenceDataset::Climatology;
        case 20:
            return TypeOfRelationToReferenceDataset::ExtremeForecastIndex;
        case 21:
            return TypeOfRelationToReferenceDataset::ShiftOfTails;
        case 22:
            return TypeOfRelationToReferenceDataset::AnomalyOfProbabilities;
        case 23:
            return TypeOfRelationToReferenceDataset::StandardizedDroughtIndex;
        case 24:
            return TypeOfRelationToReferenceDataset::CrossingPointForecast;
        case 255:
            return TypeOfRelationToReferenceDataset::Missing;
        default:
            throw Mars2GribTableException("Invalid TypeOfRelationToReferenceDataset numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..3,20..24,255}",
                                          Here());
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::tables

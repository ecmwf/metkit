// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Statistical Processing of Reference Period.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.102 – Statistical processing of reference period**
/// (GRIB key `typeOfStatisticalProcessingForTimeRangeForReferencePeriod`).
///
/// The values describe the statistical processing applied over a time range
/// of the reference period (e.g. the model climate used for EFI/SOT).
///
/// The numeric values of the enumerators map **directly** to the GRIB
/// code table and must not be modified manually.
///
/// @note
/// - This table is distinct from Code Table 4.10 (`TypeOfStatisticalProcessing`):
///   only codes `0-3` coincide.
/// - Values `5-19` and `32-191` are reserved and `192-254` are reserved for
///   local use: they are not supported.
/// - The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-level representation** only.
/// No policy, deduction, or defaulting logic must be embedded here.
///
/// @section Source of truth
/// GRIB2 Code Table 4.102 – Statistical processing of reference period,
/// as implemented in ecCodes.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and its conversion helpers automatically from
/// ecCodes GRIB code tables at build or configure time to avoid drift.
///
enum class TypeOfStatisticalProcessingForReferencePeriod : long {
    Average                                  = 0,
    Accumulation                             = 1,
    Maximum                                  = 2,
    Minimum                                  = 3,
    Median                                   = 4,
    ModelClimate                             = 20,
    NormalDistributionIndex                  = 21,
    LogNormalDistributionIndex               = 22,
    GeneralizedLogNormalDistributionIndex    = 23,
    GammaDistributionIndex                   = 24,
    LogisticDistributionIndex                = 25,
    LogLogisticDistributionIndex             = 26,
    GeneralizedLogisticDistributionIndex     = 27,
    WeibullDistributionIndex                 = 28,
    GeneralizedExtremeValueDistributionIndex = 29,
    PearsonIIIDistributionIndex              = 30,
    EmpiricalDistributionIndex               = 31,
    Missing                                  = 255
};

///
/// @brief Convert `TypeOfStatisticalProcessingForReferencePeriod` to its canonical string name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
inline std::string enum2name_TypeOfStatisticalProcessingForReferencePeriod_or_throw(
    TypeOfStatisticalProcessingForReferencePeriod value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfStatisticalProcessingForReferencePeriod::Average:
            return "average";
        case TypeOfStatisticalProcessingForReferencePeriod::Accumulation:
            return "accumulation";
        case TypeOfStatisticalProcessingForReferencePeriod::Maximum:
            return "maximum";
        case TypeOfStatisticalProcessingForReferencePeriod::Minimum:
            return "minimum";
        case TypeOfStatisticalProcessingForReferencePeriod::Median:
            return "median";
        case TypeOfStatisticalProcessingForReferencePeriod::ModelClimate:
            return "model-climate";
        case TypeOfStatisticalProcessingForReferencePeriod::NormalDistributionIndex:
            return "normal-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::LogNormalDistributionIndex:
            return "log-normal-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::GeneralizedLogNormalDistributionIndex:
            return "generalized-log-normal-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::GammaDistributionIndex:
            return "gamma-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::LogisticDistributionIndex:
            return "logistic-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::LogLogisticDistributionIndex:
            return "log-logistic-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::GeneralizedLogisticDistributionIndex:
            return "generalized-logistic-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::WeibullDistributionIndex:
            return "weibull-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::GeneralizedExtremeValueDistributionIndex:
            return "generalized-extreme-value-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::PearsonIIIDistributionIndex:
            return "pearson-iii-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::EmpiricalDistributionIndex:
            return "empirical-distribution-index";
        case TypeOfStatisticalProcessingForReferencePeriod::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid TypeOfStatisticalProcessingForReferencePeriod enum value", Here());
    }

    mars2gribUnreachable();
}

///
/// @brief Convert a canonical string name to `TypeOfStatisticalProcessingForReferencePeriod`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `TypeOfStatisticalProcessingForReferencePeriod` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
inline TypeOfStatisticalProcessingForReferencePeriod name2enum_TypeOfStatisticalProcessingForReferencePeriod_or_throw(
    const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "average") {
        return TypeOfStatisticalProcessingForReferencePeriod::Average;
    }
    if (name == "accumulation") {
        return TypeOfStatisticalProcessingForReferencePeriod::Accumulation;
    }
    if (name == "maximum") {
        return TypeOfStatisticalProcessingForReferencePeriod::Maximum;
    }
    if (name == "minimum") {
        return TypeOfStatisticalProcessingForReferencePeriod::Minimum;
    }
    if (name == "median") {
        return TypeOfStatisticalProcessingForReferencePeriod::Median;
    }
    if (name == "model-climate") {
        return TypeOfStatisticalProcessingForReferencePeriod::ModelClimate;
    }
    if (name == "normal-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::NormalDistributionIndex;
    }
    if (name == "log-normal-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::LogNormalDistributionIndex;
    }
    if (name == "generalized-log-normal-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::GeneralizedLogNormalDistributionIndex;
    }
    if (name == "gamma-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::GammaDistributionIndex;
    }
    if (name == "logistic-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::LogisticDistributionIndex;
    }
    if (name == "log-logistic-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::LogLogisticDistributionIndex;
    }
    if (name == "generalized-logistic-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::GeneralizedLogisticDistributionIndex;
    }
    if (name == "weibull-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::WeibullDistributionIndex;
    }
    if (name == "generalized-extreme-value-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::GeneralizedExtremeValueDistributionIndex;
    }
    if (name == "pearson-iii-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::PearsonIIIDistributionIndex;
    }
    if (name == "empirical-distribution-index") {
        return TypeOfStatisticalProcessingForReferencePeriod::EmpiricalDistributionIndex;
    }
    if (name == "missing") {
        return TypeOfStatisticalProcessingForReferencePeriod::Missing;
    }

    throw Mars2GribTableException("Invalid TypeOfStatisticalProcessingForReferencePeriod name: '" + name + "'", Here());

    mars2gribUnreachable();
}

///
/// @brief Convert a numeric GRIB code to `TypeOfStatisticalProcessingForReferencePeriod`.
///
/// Only values defined in **GRIB2 Code Table 4.102** are accepted:
/// `{0..4, 20..31, 255}`. Reserved and local-use values are rejected.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `TypeOfStatisticalProcessingForReferencePeriod` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a supported entry.
///
/// @important
/// This function performs **pure table validation**.
/// Any semantic interpretation must be handled by the calling deduction.
///
inline TypeOfStatisticalProcessingForReferencePeriod long2enum_TypeOfStatisticalProcessingForReferencePeriod_or_throw(
    long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfStatisticalProcessingForReferencePeriod::Average;
        case 1:
            return TypeOfStatisticalProcessingForReferencePeriod::Accumulation;
        case 2:
            return TypeOfStatisticalProcessingForReferencePeriod::Maximum;
        case 3:
            return TypeOfStatisticalProcessingForReferencePeriod::Minimum;
        case 4:
            return TypeOfStatisticalProcessingForReferencePeriod::Median;
        case 20:
            return TypeOfStatisticalProcessingForReferencePeriod::ModelClimate;
        case 21:
            return TypeOfStatisticalProcessingForReferencePeriod::NormalDistributionIndex;
        case 22:
            return TypeOfStatisticalProcessingForReferencePeriod::LogNormalDistributionIndex;
        case 23:
            return TypeOfStatisticalProcessingForReferencePeriod::GeneralizedLogNormalDistributionIndex;
        case 24:
            return TypeOfStatisticalProcessingForReferencePeriod::GammaDistributionIndex;
        case 25:
            return TypeOfStatisticalProcessingForReferencePeriod::LogisticDistributionIndex;
        case 26:
            return TypeOfStatisticalProcessingForReferencePeriod::LogLogisticDistributionIndex;
        case 27:
            return TypeOfStatisticalProcessingForReferencePeriod::GeneralizedLogisticDistributionIndex;
        case 28:
            return TypeOfStatisticalProcessingForReferencePeriod::WeibullDistributionIndex;
        case 29:
            return TypeOfStatisticalProcessingForReferencePeriod::GeneralizedExtremeValueDistributionIndex;
        case 30:
            return TypeOfStatisticalProcessingForReferencePeriod::PearsonIIIDistributionIndex;
        case 31:
            return TypeOfStatisticalProcessingForReferencePeriod::EmpiricalDistributionIndex;
        case 255:
            return TypeOfStatisticalProcessingForReferencePeriod::Missing;
        default:
            throw Mars2GribTableException(
                "Invalid TypeOfStatisticalProcessingForReferencePeriod numeric value: actual='" +
                    std::to_string(value) + "', expected={0..4,20..31,255}",
                Here());
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::tables

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
 * @brief GRIB classification of processed data products.
 *
 * This enumeration represents the GRIB code table values associated with
 * *Type of processed data* (GRIB2, Code Table 1.4).
 *
 * Each enumerator describes the nature of the data contained in the GRIB
 * message, distinguishing between analysis, forecast, ensemble components,
 * observational products, and derived or experimental datasets.
 *
 * The numeric values of the enumerators map **directly** to the GRIB
 * code table values and must not be changed manually.
 *
 * @important
 * This enum is a **GRIB-level representation**, not a policy decision.
 * All semantic validation, defaulting, and deduction logic must be handled
 * in the corresponding deduction layer.
 *
 * @note
 * The value `255` corresponds to the GRIB *missing* value.
 *
 * @section Source of truth
 * The authoritative definition of this table is maintained by WMO / ecCodes:
 *
 *   GRIB2 — Code Table 1.4 (Type of processed data)
 *
 * @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: medium]
 * - Generate this enumeration automatically from ecCodes GRIB tables
 *   at build or configure time to avoid divergence across software stacks.
 */
enum class TypeOfProcessedData : long {
    AnalysisProducts                    = 0,
    ForecastProducts                    = 1,
    AnalysisAndForecastProducts         = 2,
    ControlForecastProducts             = 3,
    PerturbedForecastProducts           = 4,
    ControlAndPerturbedForecastProducts = 5,
    ProcessedSatelliteObservations      = 6,
    ProcessedRadarObservations          = 7,
    EventProbability                    = 8,
    ExperimentalData                    = 9,
    MlBasedForecast                     = 10,
    Missing                             = 255
};

/**
 * @brief Convert a symbolic name to `TypeOfProcessedData`.
 *
 * This function maps a string identifier to the corresponding
 * `TypeOfProcessedData` enumeration value.
 *
 * The mapping is explicit and strict. Only supported names are accepted.
 *
 * @param[in] value Symbolic name of the processed data type
 *
 * @return Corresponding `TypeOfProcessedData` enumerator
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the provided name is not supported.
 *
 * @note
 * - No normalization or fallback is performed.
 * - Intended for configuration, testing, and diagnostics.
 */
inline TypeOfProcessedData name2enum_TypeOfProcessedData_or_throw(const std::string& value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (value == "analysis")
        return TypeOfProcessedData::AnalysisProducts;
    if (value == "forecast")
        return TypeOfProcessedData::ForecastProducts;
    if (value == "analysis+forecast")
        return TypeOfProcessedData::AnalysisAndForecastProducts;
    if (value == "control-forecast")
        return TypeOfProcessedData::ControlForecastProducts;
    if (value == "perturbed-forecast")
        return TypeOfProcessedData::PerturbedForecastProducts;
    if (value == "control+perturbed-forecast")
        return TypeOfProcessedData::ControlAndPerturbedForecastProducts;
    if (value == "satellite-observations")
        return TypeOfProcessedData::ProcessedSatelliteObservations;
    if (value == "radar-observations")
        return TypeOfProcessedData::ProcessedRadarObservations;
    if (value == "event-probability")
        return TypeOfProcessedData::EventProbability;
    if (value == "experimental")
        return TypeOfProcessedData::ExperimentalData;
    if (value == "ml-based-forecast")
        return TypeOfProcessedData::MlBasedForecast;
    if (value == "missing")
        return TypeOfProcessedData::Missing;

    std::string errMsg = "Invalid TypeOfProcessedData name: ";
    errMsg += "actual='" + value + "'";
    throw Mars2GribTableException(errMsg, Here());

    __builtin_unreachable();
}


/**
 * @brief Map a numeric GRIB value to `TypeOfProcessedData`.
 *
 * This function validates and converts a raw numeric GRIB value
 * associated with the `typeOfProcessedData` key into the corresponding
 * `TypeOfProcessedData` enumeration.
 *
 * The mapping is **explicit and strict**. Only numeric values defined by
 * GRIB2 Code Table 1.4 and supported by this encoder are accepted.
 * Any other value is considered invalid and results in an exception.
 *
 * @section Accepted values
 * The following mappings are supported:
 *
 * - `0`   → `AnalysisProducts`
 * - `1`   → `ForecastProducts`
 * - `2`   → `AnalysisAndForecastProducts`
 * - `3`   → `ControlForecastProducts`
 * - `4`   → `PerturbedForecastProducts`
 * - `5`   → `ControlAndPerturbedForecastProducts`
 * - `6`   → `ProcessedSatelliteObservations`
 * - `7`   → `ProcessedRadarObservations`
 * - `8`   → `EventProbability`
 * - `9`   → `ExperimentalData`
 * - `10`  → `MlBasedForecast`
 * - `255` → `Missing`
 *
 * @param[in] value Raw numeric GRIB value to be validated and mapped
 *
 * @return The corresponding `TypeOfProcessedData` enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the provided numeric value does not correspond to a supported
 *         GRIB code.
 *
 * @note
 * - This function performs **no deduction** and **no defaulting**.
 * - It must not be used to infer semantics from MARS metadata.
 * - It is intended for validation of existing GRIB state or explicit overrides.
 *
 * @important
 * This function is part of the **tables layer**.
 * Policy decisions and semantic deductions must be implemented elsewhere.
 *
 * @section Source of truth
 * WMO GRIB2 Code Table 1.4 – Type of processed data.
 *
 * @todo [owner: mival][scope: tables][reason: correctness][prio: medium]
 * - Replace this hard-coded mapping with code generated automatically
 *   from ecCodes GRIB tables to prevent divergence between software stacks.
 */
inline TypeOfProcessedData long2enum_TypeOfProcessedData_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfProcessedData::AnalysisProducts;
        case 1:
            return TypeOfProcessedData::ForecastProducts;
        case 2:
            return TypeOfProcessedData::AnalysisAndForecastProducts;
        case 3:
            return TypeOfProcessedData::ControlForecastProducts;
        case 4:
            return TypeOfProcessedData::PerturbedForecastProducts;
        case 5:
            return TypeOfProcessedData::ControlAndPerturbedForecastProducts;
        case 6:
            return TypeOfProcessedData::ProcessedSatelliteObservations;
        case 7:
            return TypeOfProcessedData::ProcessedRadarObservations;
        case 8:
            return TypeOfProcessedData::EventProbability;
        case 9:
            return TypeOfProcessedData::ExperimentalData;
        case 10:
            return TypeOfProcessedData::MlBasedForecast;
        case 255:
            return TypeOfProcessedData::Missing;
        default:
            throw Mars2GribTableException("Invalid GRIB value for `typeOfProcessedData`: " + std::to_string(value),
                                          Here());
    }

    __builtin_unreachable();
}


/**
 * @brief Convert `TypeOfProcessedData` to a symbolic name.
 *
 * This function maps a `TypeOfProcessedData` enumeration value to its
 * canonical string representation.
 *
 * @param[in] value Enumeration value
 *
 * @return Canonical string name
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the value is not recognized.
 *
 * @note
 * - Intended for logging, debugging, and diagnostics.
 * - The returned names are stable identifiers, not user-facing text.
 */
inline std::string enum2name_TypeOfProcessedData_or_throw(TypeOfProcessedData value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfProcessedData::AnalysisProducts:
            return "analysis";
        case TypeOfProcessedData::ForecastProducts:
            return "forecast";
        case TypeOfProcessedData::AnalysisAndForecastProducts:
            return "analysis+forecast";
        case TypeOfProcessedData::ControlForecastProducts:
            return "control-forecast";
        case TypeOfProcessedData::PerturbedForecastProducts:
            return "perturbed-forecast";
        case TypeOfProcessedData::ControlAndPerturbedForecastProducts:
            return "control+perturbed-forecast";
        case TypeOfProcessedData::ProcessedSatelliteObservations:
            return "satellite-observations";
        case TypeOfProcessedData::ProcessedRadarObservations:
            return "radar-observations";
        case TypeOfProcessedData::EventProbability:
            return "event-probability";
        case TypeOfProcessedData::ExperimentalData:
            return "experimental";
        case TypeOfProcessedData::MlBasedForecast:
            return "ml-based-forecast";
        case TypeOfProcessedData::Missing:
            return "missing";
    }

    throw Mars2GribTableException("Invalid TypeOfProcessedData enum value", Here());
}


}  // namespace metkit::mars2grib::backend::tables
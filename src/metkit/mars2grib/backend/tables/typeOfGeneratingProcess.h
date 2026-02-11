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
/// @brief GRIB Code Table 4.3 – Type of generating process.
///
/// This enumeration represents the GRIB2 *Type of Generating Process*
/// as defined in Section 4, Code Table 3.
///
/// The numeric values map **directly** to the official GRIB code table
/// and must not be changed manually.
///
/// @section Source of truth
/// GRIB2 Code Table 4.3:
/// Type of generating process
///
/// @note
/// This enum is a pure GRIB table representation.
/// No semantic interpretation or policy decisions are encoded here.
///
/// @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: critical]
/// - Generate this enum automatically from ecCodes GRIB tables
/// to guarantee alignment with the runtime ecCodes version.
///
enum class TypeOfGeneratingProcess : long {
    Analysis                           = 0,
    Initialization                     = 1,
    Forecast                           = 2,
    BiasCorrectedForecast              = 3,
    EnsembleForecast                   = 4,
    ProbabilityForecast                = 5,
    ForecastError                      = 6,
    AnalysisError                      = 7,
    Observation                        = 8,
    Climatological                     = 9,
    ProbabilityWeightedForecast        = 10,
    BiasCorrectedEnsembleForecast      = 11,
    PostProcessedAnalysis              = 12,
    PostProcessedForecast              = 13,
    Nowcast                            = 14,
    Hindcast                           = 15,
    PhysicalRetrieval                  = 16,
    RegressionAnalysis                 = 17,
    DifferenceBetweenTwoForecasts      = 18,
    FirstGuess                         = 19,
    AnalysisIncrement                  = 20,
    InitializationIncrementForAnalysis = 21,
    BlendedForecast                    = 22,
    Missing                            = 255
};

///
/// @brief Map a canonical string identifier to a GRIB `TypeOfGeneratingProcess`.
///
/// This function converts a string-based identifier into the corresponding
/// GRIB `TypeOfGeneratingProcess` enumeration value.
///
/// The mapping is explicit and strict. Only officially supported identifiers
/// are accepted.
///
/// @param[in] value Canonical string identifier
///
/// @return Corresponding `TypeOfGeneratingProcess` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided string does not correspond to a supported
/// generating process type.
///
/// @note
/// - This is a **pure table mapping**.
/// - No normalization, fallback, or defaulting is performed.
///
/// @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes GRIB code tables.
///
inline TypeOfGeneratingProcess name2enum_TypeOfGeneratingProcess_or_throw(const std::string& value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (value == "analysis")
        return TypeOfGeneratingProcess::Analysis;
    else if (value == "initialization")
        return TypeOfGeneratingProcess::Initialization;
    else if (value == "forecast")
        return TypeOfGeneratingProcess::Forecast;
    else if (value == "biasCorrectedForecast")
        return TypeOfGeneratingProcess::BiasCorrectedForecast;
    else if (value == "ensembleForecast")
        return TypeOfGeneratingProcess::EnsembleForecast;
    else if (value == "probabilityForecast")
        return TypeOfGeneratingProcess::ProbabilityForecast;
    else if (value == "forecastError")
        return TypeOfGeneratingProcess::ForecastError;
    else if (value == "analysisError")
        return TypeOfGeneratingProcess::AnalysisError;
    else if (value == "observation")
        return TypeOfGeneratingProcess::Observation;
    else if (value == "climatological")
        return TypeOfGeneratingProcess::Climatological;
    else if (value == "probabilityWeightedForecast")
        return TypeOfGeneratingProcess::ProbabilityWeightedForecast;
    else if (value == "biasCorrectedEnsembleForecast")
        return TypeOfGeneratingProcess::BiasCorrectedEnsembleForecast;
    else if (value == "postProcessedAnalysis")
        return TypeOfGeneratingProcess::PostProcessedAnalysis;
    else if (value == "postProcessedForecast")
        return TypeOfGeneratingProcess::PostProcessedForecast;
    else if (value == "nowcast")
        return TypeOfGeneratingProcess::Nowcast;
    else if (value == "hindcast")
        return TypeOfGeneratingProcess::Hindcast;
    else if (value == "physicalRetrieval")
        return TypeOfGeneratingProcess::PhysicalRetrieval;
    else if (value == "regressionAnalysis")
        return TypeOfGeneratingProcess::RegressionAnalysis;
    else if (value == "differenceBetweenTwoForecasts")
        return TypeOfGeneratingProcess::DifferenceBetweenTwoForecasts;
    else if (value == "firstGuess")
        return TypeOfGeneratingProcess::FirstGuess;
    else if (value == "analysisIncrement")
        return TypeOfGeneratingProcess::AnalysisIncrement;
    else if (value == "initializationIncrementForAnalysis")
        return TypeOfGeneratingProcess::InitializationIncrementForAnalysis;
    else if (value == "blendedForecast")
        return TypeOfGeneratingProcess::BlendedForecast;
    else if (value == "missing")
        return TypeOfGeneratingProcess::Missing;
    else {
        std::string errMsg = "Invalid TypeOfGeneratingProcess value: ";
        errMsg += "actual='" + value + "'";
        throw Mars2GribTableException(errMsg, Here());
    }

    mars2gribUnreachable();
}

///
/// @brief Map a GRIB `TypeOfGeneratingProcess` enumeration to its canonical string identifier.
///
/// This function converts a GRIB-level `TypeOfGeneratingProcess` enumeration
/// value into its canonical string representation.
///
/// @param[in] value GRIB `TypeOfGeneratingProcess` enumeration value
///
/// @return Canonical string identifier corresponding to the enumeration
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is not supported.
///
/// @note
/// - This function is the strict inverse of
/// `mapto_TypeOfGeneratingProcess_or_throw(const std::string&)`.
/// - No fallback or defaulting is performed.
///
/// @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes GRIB code tables.
///
inline std::string enum2name_TypeOfGeneratingProcess_or_throw(TypeOfGeneratingProcess value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfGeneratingProcess::Analysis:
            return "analysis";
        case TypeOfGeneratingProcess::Initialization:
            return "initialization";
        case TypeOfGeneratingProcess::Forecast:
            return "forecast";
        case TypeOfGeneratingProcess::BiasCorrectedForecast:
            return "biasCorrectedForecast";
        case TypeOfGeneratingProcess::EnsembleForecast:
            return "ensembleForecast";
        case TypeOfGeneratingProcess::ProbabilityForecast:
            return "probabilityForecast";
        case TypeOfGeneratingProcess::ForecastError:
            return "forecastError";
        case TypeOfGeneratingProcess::AnalysisError:
            return "analysisError";
        case TypeOfGeneratingProcess::Observation:
            return "observation";
        case TypeOfGeneratingProcess::Climatological:
            return "climatological";
        case TypeOfGeneratingProcess::ProbabilityWeightedForecast:
            return "probabilityWeightedForecast";
        case TypeOfGeneratingProcess::BiasCorrectedEnsembleForecast:
            return "biasCorrectedEnsembleForecast";
        case TypeOfGeneratingProcess::PostProcessedAnalysis:
            return "postProcessedAnalysis";
        case TypeOfGeneratingProcess::PostProcessedForecast:
            return "postProcessedForecast";
        case TypeOfGeneratingProcess::Nowcast:
            return "nowcast";
        case TypeOfGeneratingProcess::Hindcast:
            return "hindcast";
        case TypeOfGeneratingProcess::PhysicalRetrieval:
            return "physicalRetrieval";
        case TypeOfGeneratingProcess::RegressionAnalysis:
            return "regressionAnalysis";
        case TypeOfGeneratingProcess::DifferenceBetweenTwoForecasts:
            return "differenceBetweenTwoForecasts";
        case TypeOfGeneratingProcess::FirstGuess:
            return "firstGuess";
        case TypeOfGeneratingProcess::AnalysisIncrement:
            return "analysisIncrement";
        case TypeOfGeneratingProcess::InitializationIncrementForAnalysis:
            return "initializationIncrementForAnalysis";
        case TypeOfGeneratingProcess::BlendedForecast:
            return "blendedForecast";
        case TypeOfGeneratingProcess::Missing:
            return "missing";
        default:
            std::string errMsg = "Invalid TypeOfGeneratingProcess enum value: ";
            errMsg += std::to_string(static_cast<long>(value));
            throw Mars2GribTableException(errMsg, Here());
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::tables
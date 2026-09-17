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
template <class Cntx_t>
inline TypeOfGeneratingProcess name2enum_TypeOfGeneratingProcess_or_throw(const std::string& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (value == "analysis") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Analysis;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "initialization") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Initialization;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "forecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Forecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "biasCorrectedForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::BiasCorrectedForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "ensembleForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::EnsembleForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "probabilityForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::ProbabilityForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "forecastError") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::ForecastError;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "analysisError") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::AnalysisError;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "observation") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Observation;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "climatological") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Climatological;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "probabilityWeightedForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::ProbabilityWeightedForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "biasCorrectedEnsembleForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::BiasCorrectedEnsembleForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "postProcessedAnalysis") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::PostProcessedAnalysis;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "postProcessedForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::PostProcessedForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "nowcast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Nowcast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "hindcast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Hindcast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "physicalRetrieval") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::PhysicalRetrieval;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "regressionAnalysis") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::RegressionAnalysis;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "differenceBetweenTwoForecasts") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::DifferenceBetweenTwoForecasts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "firstGuess") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::FirstGuess;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "analysisIncrement") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::AnalysisIncrement;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "initializationIncrementForAnalysis") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::InitializationIncrementForAnalysis;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "blendedForecast") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::BlendedForecast;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "missing") {
        {
            TypeOfGeneratingProcess result = TypeOfGeneratingProcess::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
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
template <class Cntx_t>
inline std::string enum2name_TypeOfGeneratingProcess_or_throw(TypeOfGeneratingProcess value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfGeneratingProcess::Analysis:
            {
                std::string result = "analysis";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Initialization:
            {
                std::string result = "initialization";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Forecast:
            {
                std::string result = "forecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::BiasCorrectedForecast:
            {
                std::string result = "biasCorrectedForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::EnsembleForecast:
            {
                std::string result = "ensembleForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::ProbabilityForecast:
            {
                std::string result = "probabilityForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::ForecastError:
            {
                std::string result = "forecastError";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::AnalysisError:
            {
                std::string result = "analysisError";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Observation:
            {
                std::string result = "observation";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Climatological:
            {
                std::string result = "climatological";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::ProbabilityWeightedForecast:
            {
                std::string result = "probabilityWeightedForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::BiasCorrectedEnsembleForecast:
            {
                std::string result = "biasCorrectedEnsembleForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::PostProcessedAnalysis:
            {
                std::string result = "postProcessedAnalysis";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::PostProcessedForecast:
            {
                std::string result = "postProcessedForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Nowcast:
            {
                std::string result = "nowcast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Hindcast:
            {
                std::string result = "hindcast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::PhysicalRetrieval:
            {
                std::string result = "physicalRetrieval";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::RegressionAnalysis:
            {
                std::string result = "regressionAnalysis";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::DifferenceBetweenTwoForecasts:
            {
                std::string result = "differenceBetweenTwoForecasts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::FirstGuess:
            {
                std::string result = "firstGuess";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::AnalysisIncrement:
            {
                std::string result = "analysisIncrement";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::InitializationIncrementForAnalysis:
            {
                std::string result = "initializationIncrementForAnalysis";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::BlendedForecast:
            {
                std::string result = "blendedForecast";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfGeneratingProcess::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            std::string errMsg = "Invalid TypeOfGeneratingProcess enum value: ";
            errMsg += std::to_string(static_cast<long>(value));
            throw Mars2GribTableException(errMsg, Here());
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::tables
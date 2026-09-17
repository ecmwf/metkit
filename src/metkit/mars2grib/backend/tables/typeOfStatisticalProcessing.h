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
/// @brief GRIB Table 4.10 – Type of statistical processing.
///
/// This enumeration represents the GRIB code values used to describe
/// the statistical processing applied over a time range.
///
/// @section Source of truth
/// GRIB2 Code Table 4.10 (ecCodes)
///
/// @note
/// - Numeric values map **directly** to GRIB code table entries.
/// - Value `255` corresponds to GRIB *missing*.
///
/// @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: critical]
/// - Generate this enum and its mappings automatically from ecCodes
/// definitions at build time to avoid divergence.
///
enum class TypeOfStatisticalProcessing : long {
    Average                 = 0,
    Accumulation            = 1,
    Maximum                 = 2,
    Minimum                 = 3,
    DifferenceEndMinusStart = 4,
    RootMeanSquare          = 5,
    StandardDeviation       = 6,
    Covariance              = 7,
    DifferenceStartMinusEnd = 8,
    Ratio                   = 9,
    StandardizedAnomaly     = 10,
    Summation               = 11,
    ReturnPeriod            = 12,
    Median                  = 13,

    Severity        = 100,
    Mode            = 101,
    IndexProcessing = 102,

    Missing = 255
};

///
/// @brief Convert a symbolic name to a GRIB TypeOfStatisticalProcessing.
///
/// Supported names:
///
/// - "average"
/// - "accumulation"
/// - "maximum"
/// - "minimum"
/// - "difference_end_minus_start"
/// - "root_mean_square"
/// - "standard_deviation"
/// - "covariance"
/// - "difference_start_minus_end"
/// - "ratio"
/// - "standardized_anomaly"
/// - "summation"
/// - "return_period"
/// - "median"
/// - "severity"
/// - "mode"
/// - "index_processing"
/// - "missing"
///
/// @param[in] name Canonical symbolic name
///
/// @return Corresponding `TypeOfStatisticalProcessing`
///
/// @throws Mars2GribTableException
/// If the name is not supported
///
/// @note
/// - Case-sensitive
/// - No aliasing or normalization
///
template <class Cntx_t>
inline TypeOfStatisticalProcessing name2enum_TypeOfStatisticalProcessing_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "average") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Average;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "accumulation") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Accumulation;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "maximum") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Maximum;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "minimum") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Minimum;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "difference_end_minus_start") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::DifferenceEndMinusStart;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "root_mean_square") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::RootMeanSquare;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "standard_deviation") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::StandardDeviation;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "covariance") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Covariance;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "difference_start_minus_end") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::DifferenceStartMinusEnd;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "ratio") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Ratio;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "standardized_anomaly") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::StandardizedAnomaly;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "summation") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Summation;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "return_period") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::ReturnPeriod;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "median") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Median;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    if (name == "severity") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Severity;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "mode") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Mode;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "index_processing") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::IndexProcessing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    if (name == "missing") {
        {
            TypeOfStatisticalProcessing result = TypeOfStatisticalProcessing::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    std::string err = "Invalid TypeOfStatisticalProcessing name: actual='" + name +
                      "', expected={average,accumulation,maximum,minimum,"
                      "difference_end_minus_start,root_mean_square,standard_deviation,"
                      "covariance,difference_start_minus_end,ratio,standardized_anomaly,"
                      "summation,return_period,median,severity,mode,index_processing,missing}";
    throw Mars2GribTableException(err, Here());

    mars2gribUnreachable();
}

///
/// @brief Convert a GRIB TypeOfStatisticalProcessing to its symbolic name.
///
/// @param[in] value GRIB statistical processing code
///
/// @return Canonical symbolic name
///
/// @throws Mars2GribTableException
/// If the enum value is not supported
///
/// @note
/// - Returned strings are stable and suitable for round-tripping
///
template <class Cntx_t>
inline std::string enum2name_TypeOfStatisticalProcessing_or_throw(TypeOfStatisticalProcessing value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfStatisticalProcessing::Average:
            {
                std::string result = "average";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Accumulation:
            {
                std::string result = "accumulation";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Maximum:
            {
                std::string result = "maximum";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Minimum:
            {
                std::string result = "minimum";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::DifferenceEndMinusStart:
            {
                std::string result = "difference_end_minus_start";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::RootMeanSquare:
            {
                std::string result = "root_mean_square";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::StandardDeviation:
            {
                std::string result = "standard_deviation";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Covariance:
            {
                std::string result = "covariance";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::DifferenceStartMinusEnd:
            {
                std::string result = "difference_start_minus_end";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Ratio:
            {
                std::string result = "ratio";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::StandardizedAnomaly:
            {
                std::string result = "standardized_anomaly";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Summation:
            {
                std::string result = "summation";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::ReturnPeriod:
            {
                std::string result = "return_period";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Median:
            {
                std::string result = "median";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }

        case TypeOfStatisticalProcessing::Severity:
            {
                std::string result = "severity";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::Mode:
            {
                std::string result = "mode";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfStatisticalProcessing::IndexProcessing:
            {
                std::string result = "index_processing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }

        case TypeOfStatisticalProcessing::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
    }

    std::string err =
        "Invalid TypeOfStatisticalProcessing enum value: actual='" + std::to_string(static_cast<long>(value)) + "'";
    throw Mars2GribTableException(err, Here());

    mars2gribUnreachable();
}


}  // namespace metkit::mars2grib::backend::tables
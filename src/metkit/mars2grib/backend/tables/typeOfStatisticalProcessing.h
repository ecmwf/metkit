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
 * @brief GRIB Table 4.10 – Type of statistical processing.
 *
 * This enumeration represents the GRIB code values used to describe
 * the statistical processing applied over a time range.
 *
 * @section Source of truth
 * GRIB2 Code Table 4.10 (ecCodes)
 *
 * @note
 * - Numeric values map **directly** to GRIB code table entries.
 * - Value `255` corresponds to GRIB *missing*.
 *
 * @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: critical]
 * - Generate this enum and its mappings automatically from ecCodes
 *   definitions at build time to avoid divergence.
 */
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

/**
 * @brief Convert a symbolic name to a GRIB TypeOfStatisticalProcessing.
 *
 * Supported names:
 *
 * - "average"
 * - "accumulation"
 * - "maximum"
 * - "minimum"
 * - "difference_end_minus_start"
 * - "root_mean_square"
 * - "standard_deviation"
 * - "covariance"
 * - "difference_start_minus_end"
 * - "ratio"
 * - "standardized_anomaly"
 * - "summation"
 * - "return_period"
 * - "median"
 * - "severity"
 * - "mode"
 * - "index_processing"
 * - "missing"
 *
 * @param[in] name Canonical symbolic name
 *
 * @return Corresponding `TypeOfStatisticalProcessing`
 *
 * @throws Mars2GribTableException
 *         If the name is not supported
 *
 * @note
 * - Case-sensitive
 * - No aliasing or normalization
 */
inline TypeOfStatisticalProcessing name2enum_TypeOfStatisticalProcessing_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "average")
        return TypeOfStatisticalProcessing::Average;
    if (name == "accumulation")
        return TypeOfStatisticalProcessing::Accumulation;
    if (name == "maximum")
        return TypeOfStatisticalProcessing::Maximum;
    if (name == "minimum")
        return TypeOfStatisticalProcessing::Minimum;
    if (name == "difference_end_minus_start")
        return TypeOfStatisticalProcessing::DifferenceEndMinusStart;
    if (name == "root_mean_square")
        return TypeOfStatisticalProcessing::RootMeanSquare;
    if (name == "standard_deviation")
        return TypeOfStatisticalProcessing::StandardDeviation;
    if (name == "covariance")
        return TypeOfStatisticalProcessing::Covariance;
    if (name == "difference_start_minus_end")
        return TypeOfStatisticalProcessing::DifferenceStartMinusEnd;
    if (name == "ratio")
        return TypeOfStatisticalProcessing::Ratio;
    if (name == "standardized_anomaly")
        return TypeOfStatisticalProcessing::StandardizedAnomaly;
    if (name == "summation")
        return TypeOfStatisticalProcessing::Summation;
    if (name == "return_period")
        return TypeOfStatisticalProcessing::ReturnPeriod;
    if (name == "median")
        return TypeOfStatisticalProcessing::Median;

    if (name == "severity")
        return TypeOfStatisticalProcessing::Severity;
    if (name == "mode")
        return TypeOfStatisticalProcessing::Mode;
    if (name == "index_processing")
        return TypeOfStatisticalProcessing::IndexProcessing;

    if (name == "missing")
        return TypeOfStatisticalProcessing::Missing;

    std::string err = "Invalid TypeOfStatisticalProcessing name: actual='" + name +
                      "', expected={average,accumulation,maximum,minimum,"
                      "difference_end_minus_start,root_mean_square,standard_deviation,"
                      "covariance,difference_start_minus_end,ratio,standardized_anomaly,"
                      "summation,return_period,median,severity,mode,index_processing,missing}";
    throw Mars2GribTableException(err, Here());

    __builtin_unreachable();
}

/**
 * @brief Convert a GRIB TypeOfStatisticalProcessing to its symbolic name.
 *
 * @param[in] value GRIB statistical processing code
 *
 * @return Canonical symbolic name
 *
 * @throws Mars2GribTableException
 *         If the enum value is not supported
 *
 * @note
 * - Returned strings are stable and suitable for round-tripping
 */
inline std::string enum2name_TypeOfStatisticalProcessing_or_throw(TypeOfStatisticalProcessing value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfStatisticalProcessing::Average:
            return "average";
        case TypeOfStatisticalProcessing::Accumulation:
            return "accumulation";
        case TypeOfStatisticalProcessing::Maximum:
            return "maximum";
        case TypeOfStatisticalProcessing::Minimum:
            return "minimum";
        case TypeOfStatisticalProcessing::DifferenceEndMinusStart:
            return "difference_end_minus_start";
        case TypeOfStatisticalProcessing::RootMeanSquare:
            return "root_mean_square";
        case TypeOfStatisticalProcessing::StandardDeviation:
            return "standard_deviation";
        case TypeOfStatisticalProcessing::Covariance:
            return "covariance";
        case TypeOfStatisticalProcessing::DifferenceStartMinusEnd:
            return "difference_start_minus_end";
        case TypeOfStatisticalProcessing::Ratio:
            return "ratio";
        case TypeOfStatisticalProcessing::StandardizedAnomaly:
            return "standardized_anomaly";
        case TypeOfStatisticalProcessing::Summation:
            return "summation";
        case TypeOfStatisticalProcessing::ReturnPeriod:
            return "return_period";
        case TypeOfStatisticalProcessing::Median:
            return "median";

        case TypeOfStatisticalProcessing::Severity:
            return "severity";
        case TypeOfStatisticalProcessing::Mode:
            return "mode";
        case TypeOfStatisticalProcessing::IndexProcessing:
            return "index_processing";

        case TypeOfStatisticalProcessing::Missing:
            return "missing";
    }

    std::string err =
        "Invalid TypeOfStatisticalProcessing enum value: actual='" + std::to_string(static_cast<long>(value)) + "'";
    throw Mars2GribTableException(err, Here());

    __builtin_unreachable();
}


}  // namespace metkit::mars2grib::backend::tables
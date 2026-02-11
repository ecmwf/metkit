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

///
/// @brief GRIB significance of reference time.
///
/// This enumeration represents the GRIB code table values associated with
/// the `significanceOfReferenceTime` key in the Product Definition Section.
///
/// The significance describes the semantic meaning of the reference time
/// used in the GRIB message (e.g. analysis time, forecast start time,
/// observation time).
///
/// The numeric values map **directly** to the GRIB Code Table
/// “Significance of reference time”.
///
/// @important
/// This enum is a **pure GRIB-level representation**.
/// It does not encode policy decisions or deduction logic.
/// All semantic resolution must be implemented in dedicated deduction
/// functions.
///
/// @section Source of truth
/// GRIB2 Code Table 1.2:
/// Significance of reference time
///
/// @todo [owner: mival,mds,dgov][scope: tables][reason: correctness][prio: critical]
/// - Generate this enumeration automatically from ecCodes definitions
/// at build or configure time (e.g. via a Python code-generation step).
/// - Avoid manual duplication of GRIB tables to prevent semantic drift
/// between mars2grib and ecCodes.
///
/// @note
/// There is currently no known ecCodes API to set or retrieve these values
/// via symbolic names; numeric values must therefore be used.
///
enum class SignificanceOfReferenceTime : long {
    Analysis             = 0,
    ForecastStart        = 1,
    ForecastVerification = 2,
    ObservationTime      = 3,
    LocalTime            = 4,
    SimulationStart      = 5,
    AssimilationStart    = 6,
    Missing              = 255
};

///
/// @brief Map a canonical string identifier to `SignificanceOfReferenceTime`.
///
/// This function converts a canonical string representation into the
/// corresponding GRIB `SignificanceOfReferenceTime` enumeration value.
///
/// The mapping is explicit and strict.
///
/// Supported identifiers:
/// - `"analysis"`
/// - `"forecastStart"`
/// - `"forecastVerification"`
/// - `"observationTime"`
/// - `"localTime"`
/// - `"simulationStart"`
/// - `"missing"`
///
/// @param[in] value Canonical string identifier
///
/// @return Corresponding `SignificanceOfReferenceTime` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided string is not supported.
///
/// @note
/// - This is a **pure table mapping**.
/// - No normalization, fallback, or defaulting is performed.
///
/// @todo [owner: mival,mds,dgov][scope: tables][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes GRIB code tables.
///
inline SignificanceOfReferenceTime name2enum_SignificanceOfReferenceTime_or_throw(const std::string& value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (value == "analysis")
        return SignificanceOfReferenceTime::Analysis;
    else if (value == "forecastStart")
        return SignificanceOfReferenceTime::ForecastStart;
    else if (value == "forecastVerification")
        return SignificanceOfReferenceTime::ForecastVerification;
    else if (value == "observationTime")
        return SignificanceOfReferenceTime::ObservationTime;
    else if (value == "localTime")
        return SignificanceOfReferenceTime::LocalTime;
    else if (value == "simulationStart")
        return SignificanceOfReferenceTime::SimulationStart;
    else if (value == "assimilationStart")
        return SignificanceOfReferenceTime::SimulationStart;
    else if (value == "missing")
        return SignificanceOfReferenceTime::Missing;
    else {
        std::string errMsg = "Invalid SignificanceOfReferenceTime value: ";
        errMsg += "actual='" + value + "'";
        throw Mars2GribTableException(errMsg, Here());
    }

    __builtin_unreachable();
}

///
/// @brief Convert `SignificanceOfReferenceTime` to its canonical string identifier.
///
/// This function converts a GRIB-level `SignificanceOfReferenceTime`
/// enumeration value into its canonical string representation.
///
/// @param[in] value GRIB `SignificanceOfReferenceTime` enumeration value
///
/// @return Canonical string identifier
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is not supported.
///
/// @note
/// - This function is the strict inverse of
/// `mapto_SignificanceOfReferenceTime_or_throw(const std::string&)`.
///
/// @todo [owner: mival,mds,dgov][scope: tables][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes GRIB code tables.
///
inline std::string enum2name_SignificanceOfReferenceTime_or_throw(SignificanceOfReferenceTime value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case SignificanceOfReferenceTime::Analysis:
            return "analysis";
        case SignificanceOfReferenceTime::ForecastStart:
            return "forecastStart";
        case SignificanceOfReferenceTime::ForecastVerification:
            return "forecastVerification";
        case SignificanceOfReferenceTime::ObservationTime:
            return "observationTime";
        case SignificanceOfReferenceTime::LocalTime:
            return "localTime";
        case SignificanceOfReferenceTime::SimulationStart:
            return "simulationStart";
        case SignificanceOfReferenceTime::AssimilationStart:
            return "assimilationStart";
        case SignificanceOfReferenceTime::Missing:
            return "missing";
        default: {
            std::string errMsg = "Invalid SignificanceOfReferenceTime enum value: ";
            errMsg += std::to_string(static_cast<unsigned>(value));
            throw Mars2GribTableException(errMsg, Here());
        }
    }

    __builtin_unreachable();
}


}  // namespace metkit::mars2grib::backend::tables
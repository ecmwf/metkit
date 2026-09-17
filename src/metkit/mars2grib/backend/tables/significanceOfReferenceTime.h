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
template <class Cntx_t>
inline SignificanceOfReferenceTime name2enum_SignificanceOfReferenceTime_or_throw(const std::string& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (value == "analysis") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::Analysis;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "forecastStart") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::ForecastStart;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "forecastVerification") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::ForecastVerification;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "observationTime") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::ObservationTime;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "localTime") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::LocalTime;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "simulationStart") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::SimulationStart;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "assimilationStart") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::SimulationStart;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (value == "missing") {
        {
            SignificanceOfReferenceTime result = SignificanceOfReferenceTime::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else {
        std::string errMsg = "Invalid SignificanceOfReferenceTime value: ";
        errMsg += "actual='" + value + "'";
        throw Mars2GribTableException(errMsg, Here());
    }

    mars2gribUnreachable();
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
template <class Cntx_t>
inline std::string enum2name_SignificanceOfReferenceTime_or_throw(SignificanceOfReferenceTime value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case SignificanceOfReferenceTime::Analysis:
            {
                std::string result = "analysis";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::ForecastStart:
            {
                std::string result = "forecastStart";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::ForecastVerification:
            {
                std::string result = "forecastVerification";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::ObservationTime:
            {
                std::string result = "observationTime";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::LocalTime:
            {
                std::string result = "localTime";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::SimulationStart:
            {
                std::string result = "simulationStart";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::AssimilationStart:
            {
                std::string result = "assimilationStart";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case SignificanceOfReferenceTime::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default: {
            std::string errMsg = "Invalid SignificanceOfReferenceTime enum value: ";
            errMsg += std::to_string(static_cast<unsigned>(value));
            throw Mars2GribTableException(errMsg, Here());
        }
    }

    mars2gribUnreachable();
}


}  // namespace metkit::mars2grib::backend::tables
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
/// @brief Ensemble statistical processing type.
///
/// This enumeration represents GRIB code values defining how ensemble
/// members are statistically processed to produce the encoded field.
///
/// The numeric values map **directly** to the GRIB code table definitions
/// and must not be changed manually.
///
/// @note
/// The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-table representation only**.
/// No policy, defaulting, or deduction logic belongs here.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and all conversion helpers automatically from
/// ecCodes definitions at build time.
///
enum class DerivedForecast : long {
    UnweightedMeanAllMembers     = 0,
    WeightedMeanAllMembers       = 1,
    StdDevClusterMean            = 2,
    StdDevClusterMeanNormalized  = 3,
    SpreadAllMembers             = 4,
    LargeAnomalyIndexAllMembers  = 5,
    UnweightedMeanClusterMembers = 6,
    InterquartileRange           = 7,
    MinimumAllMembers            = 8,
    MaximumAllMembers            = 9,
    VarianceAllMembers           = 10,
    Missing                      = 255
};

///
/// @brief Convert `DerivedForecast` to its canonical name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string name
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is invalid.
///
template <class Cntx_t>
inline std::string enum2name_DerivedForecast_or_throw(DerivedForecast value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case DerivedForecast::UnweightedMeanAllMembers:
            {
                std::string result = "unweighted-mean-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::WeightedMeanAllMembers:
            {
                std::string result = "weighted-mean-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::StdDevClusterMean:
            {
                std::string result = "stddev-cluster-mean";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::StdDevClusterMeanNormalized:
            {
                std::string result = "stddev-cluster-mean-normalized";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::SpreadAllMembers:
            {
                std::string result = "spread-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::LargeAnomalyIndexAllMembers:
            {
                std::string result = "large-anomaly-index-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::UnweightedMeanClusterMembers:
            {
                std::string result = "unweighted-mean-cluster-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::InterquartileRange:
            {
                std::string result = "interquartile-range";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::MinimumAllMembers:
            {
                std::string result = "minimum-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::MaximumAllMembers:
            {
                std::string result = "maximum-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::VarianceAllMembers:
            {
                std::string result = "variance-all-members";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case DerivedForecast::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException("Invalid DerivedForecast enum value", Here());
    }
}


///
/// @brief Convert a canonical name to `DerivedForecast`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the name is not recognized.
///
template <class Cntx_t>
inline DerivedForecast name2enum_DerivedForecast_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "unweighted-mean-all-members") {
        {
            DerivedForecast result = DerivedForecast::UnweightedMeanAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "weighted-mean-all-members") {
        {
            DerivedForecast result = DerivedForecast::WeightedMeanAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "stddev-cluster-mean") {
        {
            DerivedForecast result = DerivedForecast::StdDevClusterMean;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "stddev-cluster-mean-normalized") {
        {
            DerivedForecast result = DerivedForecast::StdDevClusterMeanNormalized;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "spread-all-members") {
        {
            DerivedForecast result = DerivedForecast::SpreadAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "large-anomaly-index-all-members") {
        {
            DerivedForecast result = DerivedForecast::LargeAnomalyIndexAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "unweighted-mean-cluster-members") {
        {
            DerivedForecast result = DerivedForecast::UnweightedMeanClusterMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "interquartile-range") {
        {
            DerivedForecast result = DerivedForecast::InterquartileRange;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "minimum-all-members") {
        {
            DerivedForecast result = DerivedForecast::MinimumAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "maximum-all-members") {
        {
            DerivedForecast result = DerivedForecast::MaximumAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "variance-all-members") {
        {
            DerivedForecast result = DerivedForecast::VarianceAllMembers;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "missing") {
        {
            DerivedForecast result = DerivedForecast::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else
        throw Mars2GribTableException("Invalid DerivedForecast name: '" + name + "'", Here());
}


///
/// @brief Convert a numeric GRIB code to `DerivedForecast`.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the value is not defined in the GRIB table.
///
template <class Cntx_t>
inline DerivedForecast long2enum_DerivedForecast_or_throw(long value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            {
                DerivedForecast result = DerivedForecast::UnweightedMeanAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 1:
            {
                DerivedForecast result = DerivedForecast::WeightedMeanAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 2:
            {
                DerivedForecast result = DerivedForecast::StdDevClusterMean;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 3:
            {
                DerivedForecast result = DerivedForecast::StdDevClusterMeanNormalized;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 4:
            {
                DerivedForecast result = DerivedForecast::SpreadAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 5:
            {
                DerivedForecast result = DerivedForecast::LargeAnomalyIndexAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 6:
            {
                DerivedForecast result = DerivedForecast::UnweightedMeanClusterMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 7:
            {
                DerivedForecast result = DerivedForecast::InterquartileRange;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 8:
            {
                DerivedForecast result = DerivedForecast::MinimumAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 9:
            {
                DerivedForecast result = DerivedForecast::MaximumAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 10:
            {
                DerivedForecast result = DerivedForecast::VarianceAllMembers;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 255:
            {
                DerivedForecast result = DerivedForecast::Missing;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException(
                "Invalid DerivedForecast numeric value: actual='" + std::to_string(value) + "', expected={0..10,255}",
                Here());
    }
}


}  // namespace metkit::mars2grib::backend::tables
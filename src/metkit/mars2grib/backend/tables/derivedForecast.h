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
 * @brief Ensemble statistical processing type.
 *
 * This enumeration represents GRIB code values defining how ensemble
 * members are statistically processed to produce the encoded field.
 *
 * The numeric values map **directly** to the GRIB code table definitions
 * and must not be changed manually.
 *
 * @note
 * The value `255` corresponds to the GRIB *missing* value.
 *
 * @important
 * This enum is a **GRIB-table representation only**.
 * No policy, defaulting, or deduction logic belongs here.
 *
 * @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
 * - Generate this enum and all conversion helpers automatically from
 *   ecCodes definitions at build time.
 */
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

/**
 * @brief Convert `DerivedForecast` to its canonical name.
 *
 * @param[in] value Enumeration value
 *
 * @return Canonical string name
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the enumeration value is invalid.
 */
inline std::string enum2name_DerivedForecast_or_throw(DerivedForecast value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case DerivedForecast::UnweightedMeanAllMembers:
            return "unweighted-mean-all-members";
        case DerivedForecast::WeightedMeanAllMembers:
            return "weighted-mean-all-members";
        case DerivedForecast::StdDevClusterMean:
            return "stddev-cluster-mean";
        case DerivedForecast::StdDevClusterMeanNormalized:
            return "stddev-cluster-mean-normalized";
        case DerivedForecast::SpreadAllMembers:
            return "spread-all-members";
        case DerivedForecast::LargeAnomalyIndexAllMembers:
            return "large-anomaly-index-all-members";
        case DerivedForecast::UnweightedMeanClusterMembers:
            return "unweighted-mean-cluster-members";
        case DerivedForecast::InterquartileRange:
            return "interquartile-range";
        case DerivedForecast::MinimumAllMembers:
            return "minimum-all-members";
        case DerivedForecast::MaximumAllMembers:
            return "maximum-all-members";
        case DerivedForecast::VarianceAllMembers:
            return "variance-all-members";
        case DerivedForecast::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid DerivedForecast enum value", Here());
    }
}


/**
 * @brief Convert a canonical name to `DerivedForecast`.
 *
 * @param[in] name Canonical string name
 *
 * @return Corresponding enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the name is not recognized.
 */
inline DerivedForecast name2enum_DerivedForecast_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "unweighted-mean-all-members")
        return DerivedForecast::UnweightedMeanAllMembers;
    else if (name == "weighted-mean-all-members")
        return DerivedForecast::WeightedMeanAllMembers;
    else if (name == "stddev-cluster-mean")
        return DerivedForecast::StdDevClusterMean;
    else if (name == "stddev-cluster-mean-normalized")
        return DerivedForecast::StdDevClusterMeanNormalized;
    else if (name == "spread-all-members")
        return DerivedForecast::SpreadAllMembers;
    else if (name == "large-anomaly-index-all-members")
        return DerivedForecast::LargeAnomalyIndexAllMembers;
    else if (name == "unweighted-mean-cluster-members")
        return DerivedForecast::UnweightedMeanClusterMembers;
    else if (name == "interquartile-range")
        return DerivedForecast::InterquartileRange;
    else if (name == "minimum-all-members")
        return DerivedForecast::MinimumAllMembers;
    else if (name == "maximum-all-members")
        return DerivedForecast::MaximumAllMembers;
    else if (name == "variance-all-members")
        return DerivedForecast::VarianceAllMembers;
    else if (name == "missing")
        return DerivedForecast::Missing;
    else
        throw Mars2GribTableException("Invalid DerivedForecast name: '" + name + "'", Here());
}


/**
 * @brief Convert a numeric GRIB code to `DerivedForecast`.
 *
 * @param[in] value Numeric GRIB code
 *
 * @return Corresponding enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the value is not defined in the GRIB table.
 */
inline DerivedForecast long2enum_DerivedForecast_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return DerivedForecast::UnweightedMeanAllMembers;
        case 1:
            return DerivedForecast::WeightedMeanAllMembers;
        case 2:
            return DerivedForecast::StdDevClusterMean;
        case 3:
            return DerivedForecast::StdDevClusterMeanNormalized;
        case 4:
            return DerivedForecast::SpreadAllMembers;
        case 5:
            return DerivedForecast::LargeAnomalyIndexAllMembers;
        case 6:
            return DerivedForecast::UnweightedMeanClusterMembers;
        case 7:
            return DerivedForecast::InterquartileRange;
        case 8:
            return DerivedForecast::MinimumAllMembers;
        case 9:
            return DerivedForecast::MaximumAllMembers;
        case 10:
            return DerivedForecast::VarianceAllMembers;
        case 255:
            return DerivedForecast::Missing;
        default:
            throw Mars2GribTableException(
                "Invalid DerivedForecast numeric value: actual='" + std::to_string(value) + "', expected={0..10,255}",
                Here());
    }
}


}  // namespace metkit::mars2grib::backend::tables
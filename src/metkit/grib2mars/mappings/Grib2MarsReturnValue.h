// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file Grib2MarsReturnValue.h
/// @brief Return type used by grib2mars conversion APIs.
#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/datamod/Dict.h"

namespace metkit::grib2mars {

/// @brief Dictionary type of the auxiliary metadata for a given request dictionary type.
///
/// eckit::LocalConfiguration, except for metkit::datamod::Dict, which decodes misc into a
/// datamod::Dict as well.
template <typename MarsDict>
struct MiscDictFor {
    using type = eckit::LocalConfiguration;
};

template <>
struct MiscDictFor<metkit::datamod::Dict> {
    using type = metkit::datamod::Dict;
};

/// @brief Structured result returned by grib2mars conversions.
///
/// @tparam MarsDict
/// Dictionary type used for the converted request.
///
/// @tparam MiscDict
/// Dictionary type used for the auxiliary metadata (see MiscDictFor).
template <typename MarsDict, typename MiscDict = typename MiscDictFor<MarsDict>::type>
struct Grib2MarsResult {
    /// @brief Converted MARS request.
    MarsDict mars;

    /// @brief Auxiliary metadata produced during conversion.
    MiscDict misc;
};

}  // namespace metkit::grib2mars

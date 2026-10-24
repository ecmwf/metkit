// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file Mars2MarsReturnValue.h
/// @brief Return type used by mars2mars conversion APIs.
#pragma once

#include "eckit/config/LocalConfiguration.h"

namespace metkit::mars2mars {

/// @brief Structured result returned by mars2mars conversions.
///
/// @tparam MarsDict
/// Dictionary type used for the converted request.
template <typename MarsDict>
struct Mars2MarsResult {
    /// @brief Converted MARS request.
    MarsDict mars;

    /// @brief Auxiliary metadata produced during conversion.
    eckit::LocalConfiguration misc;
};

}  // namespace metkit::mars2mars

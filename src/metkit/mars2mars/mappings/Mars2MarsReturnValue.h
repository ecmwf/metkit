/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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

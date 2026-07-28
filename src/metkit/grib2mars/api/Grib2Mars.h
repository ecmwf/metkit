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

///
/// @file Grib2Mars.h
/// @brief Public conversion API for transforming MARS requests.
///
/// This header exposes the primary grib2mars entry point. Callers provide a
/// supported dictionary type and receive a structured result containing the
/// converted MARS request plus auxiliary conversion metadata.
///
/// The API is intentionally narrow and explicit:
/// - only supported dictionary types may be converted
/// - conversion is delegated to the rule engine
/// - all failures are routed through the shared API error boundary
///
/// @section References
/// Implementation:
/// - @ref Grib2Mars.cc
///
/// Result type:
/// - @ref Grib2MarsReturnValue.h
///
/// @ingroup grib2mars_api
///

#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars/MarsRequest.h"

// Codes wrapper types
#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/mappings/Grib2MarsReturnValue.h"

namespace metkit::grib2mars {


///
/// @brief Main API for converting MARS requests.
///
/// The converter owns no runtime configuration state. It is a thin façade
/// around the conversion rules and the shared API error handling layer.
///
class Grib2Mars {
public:

    ///
    /// @brief Construct a Grib2Mars converter.
    ///
    /// No configuration is stored in the object itself.
    ///
    Grib2Mars();

    ~Grib2Mars() = default;

    /// @brief Convert a supported dictionary type.
    ///
    /// The primary template is deleted intentionally. Only explicit
    /// specializations are available through the public API.
    ///
    /// @tparam Dict_t
    /// Input dictionary type.
    ///
    /// @param[in] mars
    /// Input MARS dictionary.
    ///
    /// @return
    /// A converted result for the supported dictionary type.
    template <typename Dict_t>
    Grib2MarsResult<Dict_t> convert(const metkit::codes::CodesHandle& grib) = delete;
};


// -----------------------------------------------------------------------------
// Supported API specializations
// -----------------------------------------------------------------------------

/// @brief Convert an `eckit::LocalConfiguration` request.
template <>
Grib2MarsResult<eckit::LocalConfiguration> Grib2Mars::convert<eckit::LocalConfiguration>(
    const metkit::codes::CodesHandle& grib);

/// @brief Convert a `metkit::mars::MarsRequest` request.
template <>
Grib2MarsResult<metkit::mars::MarsRequest> Grib2Mars::convert<metkit::mars::MarsRequest>(
    const metkit::codes::CodesHandle& grib);

}  // namespace metkit::grib2mars

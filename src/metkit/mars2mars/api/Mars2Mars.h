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
/// @file Mars2Mars.h
/// @brief Public conversion API for transforming MARS requests.
///
/// This header exposes the primary mars2mars entry point. Callers provide a
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
/// - @ref Mars2Mars.cc
///
/// Result type:
/// - @ref Mars2MarsReturnValue.h
///
/// @ingroup mars2mars_api
///

#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"

namespace metkit::mars2mars {


///
/// @brief Main API for converting MARS requests.
///
/// The converter owns no runtime configuration state. It is a thin façade
/// around the conversion rules and the shared API error handling layer.
///
class Mars2Mars {
public:

    ///
    /// @brief Construct a Mars2Mars converter.
    ///
    /// No configuration is stored in the object itself.
    ///
    Mars2Mars();

    ~Mars2Mars() = default;

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
    Mars2MarsResult<Dict_t> convert(const Dict_t& mars) = delete;
};


// -----------------------------------------------------------------------------
// Supported API specializations
// -----------------------------------------------------------------------------

/// @brief Convert an `eckit::LocalConfiguration` request.
template <>
Mars2MarsResult<eckit::LocalConfiguration> Mars2Mars::convert<eckit::LocalConfiguration>(
    const eckit::LocalConfiguration& mars);

/// @brief Convert a `metkit::mars::MarsRequest` request.
template <>
Mars2MarsResult<metkit::mars::MarsRequest> Mars2Mars::convert<metkit::mars::MarsRequest>(
    const metkit::mars::MarsRequest& mars);

}  // namespace metkit::mars2mars

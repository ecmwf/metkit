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

#include <initializer_list>
#include <string>
#include <utility>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/value/Value.h"
#include "metkit/mars/MarsRequest.h"

// Codes wrapper types
#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/api/Options.h"
#include "metkit/grib2mars/mappings/Grib2MarsReturnValue.h"

namespace metkit::grib2mars {


///
/// @brief Main API for converting MARS requests.
///
/// The converter stores a fixed runtime options snapshot. It is a thin façade
/// around the conversion rules and the shared API error handling layer.
///
class Grib2Mars {
public:

    using OptionEntry = std::pair<std::string, eckit::Value>;
    using OptionList  = std::initializer_list<OptionEntry>;

    ///
    /// @brief Construct a Grib2Mars converter with default options.
    ///
    /// Default options correspond to standard grib2mars behavior.
    ///
    Grib2Mars();

    ///
    /// @brief Construct a Grib2Mars converter with explicit options.
    ///
    /// @param[in] opts
    /// API options controlling error-stack persistence and diagnostics.
    ///
    explicit Grib2Mars(const Options& opts);

    ///
    /// @brief Construct a Grib2Mars converter from a configuration object.
    ///
    /// This constructor allows options to be provided via an
    /// `eckit::LocalConfiguration`, typically originating from YAML
    /// or JSON configuration files.
    ///
    /// @param[in] opts
    /// Configuration object describing converter options.
    ///
    explicit Grib2Mars(const eckit::LocalConfiguration& opts);

    ///
    /// @brief Construct a Grib2Mars converter from inline option entries.
    ///
    /// This constructor supports compact inline configuration such as:
    ///
    /// @code
    /// Grib2Mars{{"printErrorStackToStdErr", true}, {"saveErrorStack", true}}
    /// @endcode
    ///
    /// @param[in] opts
    /// Initializer-list option entries.
    ///
    explicit Grib2Mars(OptionList opts);

    Grib2Mars(const Grib2Mars&)           = delete;
    Grib2Mars(Grib2Mars&&)                = delete;
    Grib2Mars operator=(const Grib2Mars&) = delete;
    Grib2Mars operator=(Grib2Mars&&)      = delete;

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

private:

    const Options opts_;
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

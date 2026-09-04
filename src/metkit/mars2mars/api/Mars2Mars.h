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

#include <initializer_list>
#include <string>
#include <utility>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/value/Value.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars2mars/api/Options.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"

namespace metkit::mars2mars {


///
/// @brief Main API for converting MARS requests.
///
/// The converter stores a fixed runtime options snapshot. It is a thin façade
/// around the conversion rules and the shared API error handling layer.
///
class Mars2Mars {
public:

    using OptionEntry = std::pair<std::string, eckit::Value>;
    using OptionList  = std::initializer_list<OptionEntry>;

    ///
    /// @brief Construct a Mars2Mars converter with default options.
    ///
    /// Default options correspond to standard mars2mars behavior.
    ///
    Mars2Mars();

    ///
    /// @brief Construct a Mars2Mars converter with explicit options.
    ///
    /// @param[in] opts
    /// API options controlling error-stack persistence and diagnostics.
    ///
    explicit Mars2Mars(const Options& opts);

    ///
    /// @brief Construct a Mars2Mars converter from a configuration object.
    ///
    /// This constructor allows options to be provided via an
    /// `eckit::LocalConfiguration`, typically originating from YAML
    /// or JSON configuration files.
    ///
    /// @param[in] opts
    /// Configuration object describing converter options.
    ///
    explicit Mars2Mars(const eckit::LocalConfiguration& opts);

    ///
    /// @brief Construct a Mars2Mars converter from inline option entries.
    ///
    /// This constructor supports compact inline configuration such as:
    ///
    /// @code
    /// Mars2Mars{{"saveErrorStack", true}, {"errorStackPath", "./errors"}}
    /// @endcode
    ///
    /// @param[in] opts
    /// Initializer-list option entries.
    ///
    explicit Mars2Mars(OptionList opts);

    Mars2Mars(const Mars2Mars&)           = delete;
    Mars2Mars(Mars2Mars&&)                = delete;
    Mars2Mars operator=(const Mars2Mars&) = delete;
    Mars2Mars operator=(Mars2Mars&&)      = delete;

    ~Mars2Mars() = default;

    /// @brief Convert a MARS description
    ///
    /// @param[in] mars
    /// Input MARS dictionary.
    ///
    /// @return
    /// A converted MARS + misc result
    Mars2MarsResult<eckit::LocalConfiguration> convert(const eckit::LocalConfiguration& mars);

    /// @brief Convert a MARS description
    ///
    /// @param[in] mars
    /// Input MARS dictionary.
    ///
    /// @param[in] misc
    /// Input misc dictionary.
    ///
    /// @return
    /// A converted MARS + misc result
    Mars2MarsResult<eckit::LocalConfiguration> convert(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc);

    /// @brief Convert a MARS description
    ///
    /// @param[in] mars
    /// Input MARS dictionary.
    ///
    /// @return
    /// A converted MARS + misc result
    Mars2MarsResult<metkit::mars::MarsRequest> convert(const metkit::mars::MarsRequest& mars);

    /// @brief Convert a MARS description
    ///
    /// @param[in] mars
    /// Input MARS dictionary.
    ///
    /// @param[in] misc
    /// Input misc dictionary.
    ///
    /// @return
    /// A converted MARS + misc result
    Mars2MarsResult<metkit::mars::MarsRequest> convert(const metkit::mars::MarsRequest& mars,
                                                       const eckit::LocalConfiguration& misc);

private:

    const Options opts_;
};

}  // namespace metkit::mars2mars

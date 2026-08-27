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
/// @file Mars2Mars.cc
/// @brief Implementation of the public Mars2Mars conversion API.
///
/// The implementation forwards each supported conversion through the rule
/// layer and wraps the result in the common API error-handling policy.
///


#include "Mars2Mars.h"

#include <cstdlib>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars2mars/mappings/mappings.h"

// dictionary access traits
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_options.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"

// Error handling utilities
#include "Mars2MarsApiErrorHandling.h"
#include "metkit/mars2mars/api/readOptionsFromInitializerList.h"
#include "metkit/mars2mars/api/readOptionsFromLocalConfiguration.h"

namespace metkit::mars2mars {

namespace {

/// @brief Apply environment side effects implied by a set of options.
///
/// When `skipSection3` is enabled the encoder delegates geometry handling to
/// gridSpec/ecCodes, which requires ecCodes to be configured with eckit_geo
/// support enabled. This is controlled by the `ECCODES_ECKIT_GEO` environment
/// variable, so force it to "1" whenever `skipSection3` is requested.
inline void applyOptionSideEffects(const Options& opts) {
    if (opts.skipSection3) {
        ::setenv("ECCODES_ECKIT_GEO", "1", 1);
    }
}

}  // namespace

// -----------------------------------------------------------------------------
// Mars2Mars construction
// -----------------------------------------------------------------------------

/// @brief Default construct a Mars2Mars converter.
Mars2Mars::Mars2Mars() : opts_{} {
    applyOptionSideEffects(opts_);
}

Mars2Mars::Mars2Mars(const Options& opts) : opts_{opts} {
    applyOptionSideEffects(opts_);
}

Mars2Mars::Mars2Mars(const eckit::LocalConfiguration& opts) : opts_{detail::readOptions(opts)} {
    applyOptionSideEffects(opts_);
}

Mars2Mars::Mars2Mars(OptionList opts) : opts_{detail::readOptions(opts)} {
    applyOptionSideEffects(opts_);
}

/// @brief Convert an `eckit::LocalConfiguration` request.
template <>
Mars2MarsResult<eckit::LocalConfiguration> Mars2Mars::convert<eckit::LocalConfiguration>(
    const eckit::LocalConfiguration& mars) {

    using metkit::mars2mars::utils::exceptions::withMars2MarsApiErrorHandling;

    return withMars2MarsApiErrorHandling<Mars2MarsResult<eckit::LocalConfiguration>>(
        "Mars2Mars::convert<eckit::LocalConfiguration>", opts_,
        [&]() { return rules::convertAll<eckit::LocalConfiguration, eckit::LocalConfiguration>(mars, opts_); }, Here());
}

/// @brief Convert a `metkit::mars::MarsRequest` request.
template <>
Mars2MarsResult<metkit::mars::MarsRequest> Mars2Mars::convert<metkit::mars::MarsRequest>(
    const metkit::mars::MarsRequest& mars) {

    using metkit::mars2mars::utils::exceptions::withMars2MarsApiErrorHandling;

    return withMars2MarsApiErrorHandling<Mars2MarsResult<metkit::mars::MarsRequest>>(
        "Mars2Mars::convert<metkit::mars::MarsRequest>", opts_,
        [&]() { return rules::convertAll<metkit::mars::MarsRequest, metkit::mars::MarsRequest>(mars, opts_); }, Here());
}

}  // namespace metkit::mars2mars

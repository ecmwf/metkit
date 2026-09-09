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
/// @file Grib2Mars.cc
/// @brief Implementation of the public Grib2Mars conversion API.
///
/// The implementation forwards each supported conversion through the rule
/// layer and wraps the result in the common API error-handling policy.
///


#include "Grib2Mars.h"

#include <cstdlib>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/grib2mars/mappings/mappings.h"
#include "metkit/mars/MarsRequest.h"

// dictionary access traits
#include "metkit/grib2mars/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictaccess_options.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"

// Error handling utilities
#include "Grib2MarsApiErrorHandling.h"
#include "metkit/grib2mars/api/readOptionsFromInitializerList.h"
#include "metkit/grib2mars/api/readOptionsFromLocalConfiguration.h"

namespace metkit::grib2mars {

namespace {

/// @brief Check if the environment variables are setup correctly at runtime.
///
/// When `skipSection3` is enabled the encoder delegates geometry handling to
/// gridSpec/ecCodes, which requires ecCodes to be configured with eckit_geo
/// support enabled. This is controlled by the `ECCODES_ECKIT_GEO` environment
/// variable.
Options checkEnvironment(Options opts) {
    const auto eccodesEckitGeoValue = []() {
        const auto* eccodesEckitGeo = ::getenv("ECCODES_ECKIT_GEO");
        if (eccodesEckitGeo) {
            const std::string eccodesEckitGeoValue(eccodesEckitGeo);
            if (eccodesEckitGeoValue == "1" || eccodesEckitGeoValue == "2") {
                return true;
            }
        }
        return false;
    }();

    if (opts.skipSection3 && !eccodesEckitGeoValue) {
        throw eckit::UserError(
            "Environment variable `ECCODES_ECKIT_GEO` must be set to \"1\" or \"2\" when option `skipSection3` is "
            "enabled!",
            Here());
    }

    if (!opts.skipSection3 && eccodesEckitGeoValue) {
        opts.skipSection3 = true;
    }

    return opts;
}

}  // namespace

// -----------------------------------------------------------------------------
// Grib2Mars construction
// -----------------------------------------------------------------------------

/// @brief Default construct a Grib2Mars converter.
Grib2Mars::Grib2Mars() : opts_{checkEnvironment({})} {}

Grib2Mars::Grib2Mars(const Options& opts) : opts_{checkEnvironment(opts)} {}

Grib2Mars::Grib2Mars(const eckit::LocalConfiguration& opts) : opts_{checkEnvironment(detail::readOptions(opts))} {}

Grib2Mars::Grib2Mars(OptionList opts) : opts_{checkEnvironment(detail::readOptions(opts))} {}


/// @brief Convert an `eckit::LocalConfiguration` request.
template <>
Grib2MarsResult<eckit::LocalConfiguration> Grib2Mars::convert<eckit::LocalConfiguration>(
    const metkit::codes::CodesHandle& grib) {

    using metkit::grib2mars::utils::exceptions::withGrib2MarsApiErrorHandling;

    return withGrib2MarsApiErrorHandling<Grib2MarsResult<eckit::LocalConfiguration>>(
        "Grib2Mars::convert<eckit::LocalConfiguration>", opts_,
        [&]() { return rules::convertAll<eckit::LocalConfiguration>(grib, opts_); }, Here());
}

/// @brief Convert a `metkit::mars::MarsRequest` request.
template <>
Grib2MarsResult<metkit::mars::MarsRequest> Grib2Mars::convert<metkit::mars::MarsRequest>(
    const metkit::codes::CodesHandle& grib) {

    using metkit::grib2mars::utils::exceptions::withGrib2MarsApiErrorHandling;

    return withGrib2MarsApiErrorHandling<Grib2MarsResult<metkit::mars::MarsRequest>>(
        "Grib2Mars::convert<metkit::mars::MarsRequest>", opts_,
        [&]() { return rules::convertAll<metkit::mars::MarsRequest>(grib, opts_); }, Here());
}

}  // namespace metkit::grib2mars

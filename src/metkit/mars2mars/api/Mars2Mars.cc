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
/// @brief Implementation of the Mars2Mars converter API.
///


#include "Mars2Mars.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars2mars/mappings/all.h"

// dictionary access traits
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"

// Error handling utilities
#include "Mars2MarsApiErrorHandling.h"

namespace metkit::mars2mars {

// -----------------------------------------------------------------------------
// Mars2Mars construction
// -----------------------------------------------------------------------------

Mars2Mars::Mars2Mars() {}

template <>
Mars2MarsResult<eckit::LocalConfiguration> Mars2Mars::convert<eckit::LocalConfiguration>(
    const eckit::LocalConfiguration& mars) {

    using metkit::mars2mars::utils::exceptions::withMars2MarsApiErrorHandling;

    return withMars2MarsApiErrorHandling<Mars2MarsResult<eckit::LocalConfiguration>>(
        "Mars2Mars::convert<eckit::LocalConfiguration>",
        [&]() { return rules::convertAll<eckit::LocalConfiguration, eckit::LocalConfiguration>(mars); }, Here());
}

template <>
Mars2MarsResult<metkit::mars::MarsRequest> Mars2Mars::convert<metkit::mars::MarsRequest>(
    const metkit::mars::MarsRequest& mars) {

    using metkit::mars2mars::utils::exceptions::withMars2MarsApiErrorHandling;

    return withMars2MarsApiErrorHandling<Mars2MarsResult<metkit::mars::MarsRequest>>(
        "Mars2Mars::convert<metkit::mars::MarsRequest>",
        [&]() { return rules::convertAll<metkit::mars::MarsRequest, metkit::mars::MarsRequest>(mars); }, Here());
}

}  // namespace metkit::mars2mars

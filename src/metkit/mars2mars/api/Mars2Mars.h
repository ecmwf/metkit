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
/// @brief API for converting pre-MTG2 MARS to post-MTG2 MARS descriptions
///

#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"

namespace metkit::mars2mars {


///
/// @brief API for converting pre-MTG2 MARS to post-MTG2 MARS descriptions
///
class Mars2Mars {
public:

    ///
    /// @brief Construct a Mars2Mars converter
    ///
    Mars2Mars();

    ~Mars2Mars() = default;

    template <typename Dict_t>
    Mars2MarsResult<Dict_t> convert(const Dict_t& mars) = delete;
};


// -----------------------------------------------------------------------------
// Supported API specializations
// -----------------------------------------------------------------------------

template <>
Mars2MarsResult<eckit::LocalConfiguration> Mars2Mars::convert<eckit::LocalConfiguration>(
    const eckit::LocalConfiguration& mars);

template <>
Mars2MarsResult<metkit::mars::MarsRequest> Mars2Mars::convert<metkit::mars::MarsRequest>(
    const metkit::mars::MarsRequest& mars);

}  // namespace metkit::mars2mars

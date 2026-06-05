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
/// @file Mars2mars.cc
/// @brief Implementation of the Mars2mars converter API.
///


#include "Mars2mars.h"

// other libraries
#include "eckit/exception/Exceptions.h"

// dictionary access traits
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"

// encode header/values implementation
#include "metkit/mars2mars/CoreOperations.h"

namespace metkit::mars2mars {



// -----------------------------------------------------------------------------
// Mars2mars construction
// -----------------------------------------------------------------------------

Mars2mars::Mars2mars() {}


// -----------------------------------------------------------------------------
// Conversion interfaces
// -----------------------------------------------------------------------------
eckit::LocalConfiguration Mars2mars::convert(const eckit::LocalConfiguration& mars) {
    return CoreOperations::convert<eckit::LocalConfiguration>(mars);
}

metkit::mars::MarsRequest Mars2mars::convert(const metkit::mars::MarsRequest& mars){
    return CoreOperations::convert<metkit::mars::MarsRequest>(mars);
};

}  // namespace metkit::mars2mars

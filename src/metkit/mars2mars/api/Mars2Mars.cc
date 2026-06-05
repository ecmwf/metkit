/*
 * (C) Copyright 2025- ECMWF and individual contributors.
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
/// @brief Implementation of the Mars2mars public encoding API.
///
/// This file contains the implementation of the `Mars2mars` class declared
/// in `Mars2mars.h`.
///
/// It provides the concrete orchestration logic that:
/// - builds the internal encoder configuration from the MARS dictionary
/// - invokes the specialized backend encoder
/// - injects field values into the resulting GRIB handle
/// - exposes a temporary staged-encoding cache interface for benchmarking
///   and transitional comparison purposes
///
/// This file intentionally contains **no GRIB semantics** and **no deduction
/// logic**. All domain-specific decisions are delegated to lower layers.
///
/// ---
///
/// ## Error propagation
///
/// The current implementation propagates all exceptions across the API
/// boundary after printing extended diagnostic information.
///
/// This behavior is intentional for early integration phases and may be
/// revised in the future to provide API-stable error wrapping.
///
/// ---
///
/// ## Transitional staged API
///
/// This implementation also exposes a temporary staged-encoding interface
/// based on `prepare()` and `finaliseEncoding()`.
///
/// This interface is not intended for general use. It exists only to allow
/// short-term benchmarking and comparison against a legacy cache-based
/// implementation before the cache lifecycle is fully internalized in
/// lower layers.
///
/// ---
///
/// ## Scope
///
/// - This file is part of the **Mars2mars public API implementation**
/// - It is not intended for direct use by end users
/// - Its behavior defines the observable semantics of `Mars2mars::encode`
///   and the temporary staged-encoding API
///
/// @ingroup mars2mars_api
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

Mars2mars::Mars2mars() : opts_{} {}

Mars2mars::Mars2mars(const Options& opts) : opts_{opts} {}

Mars2mars::Mars2mars(const eckit::LocalConfiguration& opts) : opts_{readOptions(opts)} {}


// -----------------------------------------------------------------------------
// Encoding interfaces
// -----------------------------------------------------------------------------
eckit::LocalConfiguration Mars2mars::convert(const eckit::LocalConfiguration& mars) {
    return CoreOperations::convert<eckit::LocalConfiguration>(mars);
}

metkit::mars::MarsRequest Mars2mars::convert(const metkit::mars::MarsRequest& mars){
    return CoreOperations::convert<metkit::mars::MarsRequest>(mars);
};

}  // namespace metkit::mars2mars

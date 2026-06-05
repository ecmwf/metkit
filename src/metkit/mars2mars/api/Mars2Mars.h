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
/// @file Mars2mars.h
/// @brief High-level API for encoding MARS fields into GRIB messages.
///
/// This header defines the **public Mars2mars encoding API**, providing a
/// user-facing interface to convert MARS-style metadata and field values
/// into GRIB messages.
///
/// The `Mars2mars` class acts as a **stateless encoder facade**:
/// - it validates and interprets user-provided metadata
/// - it constructs a GRIB header according to MARS conventions
/// - it encodes the provided field values
/// - it returns a fully-formed GRIB handle
///
/// This API is designed for:
/// - application developers
/// - workflow orchestration layers
/// - bindings (Fortran, Python, etc.)
///
/// It intentionally hides all internal concepts such as planners,
/// deductions, sections, or encoding strategies.
///
/// ---
///
/// ## Conceptual overview
///
/// Encoding is driven by three inputs:
///
/// - **MARS dictionary** (`mars`)
/// Describes the field semantics (e.g. parameter, level, step, date).
///
/// - **Misc dictionary** (`misc`, optional)
/// Provides auxiliary metadata not strictly part of the MARS request
/// (e.g. grid geometry, packing hints, implementation options).
///
/// - **Values**
/// The numerical field values to be encoded.
///
/// The result of an encoding operation is a
/// `metkit::codes::CodesHandle`, which can be:
/// - written to file
/// - passed to ecCodes
/// - transferred to downstream systems
///
/// ---
///
/// ## Error handling
///
/// - All encoding failures are reported via C++ exceptions.
/// - Errors are fail-fast and no partial GRIB messages are produced.
/// - On failure, no `CodesHandle` is returned.
///
/// ---
///
/// ## Thread safety
///
/// - A `Mars2mars` instance is safe to use from a single thread.
/// - Concurrent use from multiple threads requires separate instances.
///
/// @ingroup mars2mars_api
///
#pragma once

// System includes
#include <memory>
#include <vector>

// eckit
#include "eckit/config/LocalConfiguration.h"
#include "eckit/value/Value.h"

// ecCodes API wrapper
#include "metkit/codes/api/CodesAPI.h"

// Codes wrapper types
#include "metkit/codes/api/CodesTypes.h"

// mars2mars public options
#include "metkit/mars2mars/api/Options.h"

namespace metkit::mars2mars {

/// ---
///
/// ## Transitional staged API
///
/// A temporary staged-encoding interface is also exposed through
/// `prepare()` and `finaliseEncoding()`.
///
/// This interface exists only for short-term benchmarking and migration
/// purposes and is not intended for external use. It may be changed or
/// removed without notice.
///


///
/// @brief High-level encoder for converting MARS fields to GRIB.
///
/// The `Mars2mars` class provides a **single-entry-point API**
/// for encoding numerical field data together with MARS metadata
/// into a GRIB message.
///
/// A `Mars2mars` object encapsulates a fixed set of encoding options
/// and can be reused to encode multiple fields with the same
/// configuration.
///
/// ### Lifetime and ownership
///
/// - `Mars2mars` does not own any external resources.
/// - Each call to `encode()` returns a new `CodesHandle` owned
/// by the caller.
///
/// ### Copy semantics
///
/// Copy and move operations are explicitly disabled to avoid
/// accidental sharing of internal state.
///
class Mars2mars {
public:

    ///
    /// @brief Construct a Mars2mars encoder with default options.
    ///
    /// Default options correspond to standard mars2mars behavior.
    ///
    Mars2mars();

    ~Mars2mars() = default;

    eckit::LocalConfiguration convert(const eckit::LocalConfiguration& mars);

    metkit::mars::MarsRequest convert(const metkit::mars::MarsRequest& mars);

};

}  // namespace metkit::mars2mars

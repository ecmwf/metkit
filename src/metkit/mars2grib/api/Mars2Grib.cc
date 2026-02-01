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

/**
 * @file Mars2Grib.cc
 * @brief Implementation of the Mars2Grib public encoding API.
 *
 * This file contains the implementation of the `Mars2Grib` class declared
 * in `Mars2Grib.h`.
 *
 * It provides the concrete orchestration logic that:
 * - builds the internal encoder configuration from the MARS dictionary
 * - invokes the specialized backend encoder
 * - injects field values into the resulting GRIB handle
 *
 * This file intentionally contains **no GRIB semantics** and **no deduction
 * logic**. All domain-specific decisions are delegated to lower layers.
 *
 * ---
 *
 * ## Error propagation
 *
 * The current implementation propagates all exceptions across the API
 * boundary after printing extended diagnostic information.
 *
 * This behavior is intentional for early integration phases and may be
 * revised in the future to provide API-stable error wrapping.
 *
 * ---
 *
 * ## Scope
 *
 * - This file is part of the **Mars2Grib public API implementation**
 * - It is not intended for direct use by end users
 * - Its behavior defines the observable semantics of `Mars2Grib::encode`
 *
 * @ingroup mars2grib_api
 */


#include "Mars2Grib.h"

// System includes
#include <iostream>
#include <limits>
#include <regex>
#include <utility>

// eckit
#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"

// ecCodes API
#include "metkit/codes/api/CodesAPI.h"

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// error handling
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib {
namespace impl {

/**
 * @brief Read Mars2Grib options from a configuration object.
 *
 * This helper function maps a subset of keys from an
 * `eckit::LocalConfiguration` into a strongly typed `Options` object.
 *
 * Only explicitly present keys are applied; all others retain their
 * default values.
 *
 * @param[in] conf
 *   Configuration object containing encoder options.
 *
 * @return
 *   A fully initialized `Options` structure.
 */
Options readOptions(const eckit::LocalConfiguration& conf) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    Options opts;
    if (has<bool>(conf, "applyChecks")) {
        opts.applyChecks = get_or_throw<bool>(conf, "applyChecks");
    }
    if (has<bool>(conf, "enableOverride")) {
        opts.enableOverride = get_or_throw<bool>(conf, "enableOverride");
    }
    if (has<bool>(conf, "enableBitsPerValueCompression")) {
        opts.enableBitsPerValueCompression = get_or_throw<bool>(conf, "enableBitsPerValueCompression");
    }
    if (has<bool>(conf, "sanitizeMars")) {
        opts.enableBitsPerValueCompression = get_or_throw<bool>(conf, "sanitizeMars");
    }
    if (has<bool>(conf, "sanitizeMisc")) {
        opts.enableBitsPerValueCompression = get_or_throw<bool>(conf, "sanitizeMisc");
    }
    if (has<bool>(conf, "fixMarsGrid")) {
        opts.enableBitsPerValueCompression = get_or_throw<bool>(conf, "fixMarsGrid");
    }
    return opts;
}

}  // namespace impl


// -----------------------------------------------------------------------------
// Mars2Grib construction
// -----------------------------------------------------------------------------

Mars2Grib::Mars2Grib() : opts_{} {}

Mars2Grib::Mars2Grib(const Options& opts) : opts_{opts} {}

Mars2Grib::Mars2Grib(const eckit::LocalConfiguration& opts) : opts_{impl::readOptions(opts)} {}


// -----------------------------------------------------------------------------
// Encoding interfaces
// -----------------------------------------------------------------------------
std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode_impl<double>( Span<const double>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {

    return encode_impl<float>( Span<const float>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode_impl<double>( Span<const double>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode_impl<float>( Span<const float>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode_impl<double>( Span<const double>{values,length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode_impl<float>( Span<const float>{values,length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode_impl<double>( Span<const double>{values,length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode_impl<float>( Span<const float>{values,length}, mars, misc);
}

}  // namespace metkit::mars2grib

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
/// @file Mars2Grib.cc
/// @brief Implementation of the Mars2Grib public encoding API.
///
/// This file contains the implementation of the `Mars2Grib` class declared
/// in `Mars2Grib.h`.
///
/// It provides the concrete orchestration logic that:
/// - builds the internal encoder configuration from the MARS dictionary
/// - invokes the specialized backend encoder
/// - injects field values into the resulting GRIB handle
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
/// ## Scope
///
/// - This file is part of the **Mars2Grib public API implementation**
/// - It is not intended for direct use by end users
/// - Its behavior defines the observable semantics of `Mars2Grib::encode`
///
/// @ingroup mars2grib_api
///


#include "Mars2Grib.h"

// System includes
#include <utility>

// eckit
#include "eckit/config/LocalConfiguration.h"

// ecCodes API
#include "metkit/codes/api/CodesAPI.h"

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// encode header/values implementation
#include "metkit/mars2grib/CoreOperations.h"

// error handling
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib {
namespace {

///
/// @brief Read Mars2Grib options from a configuration object.
///
/// This helper function maps a subset of keys from an
/// `eckit::LocalConfiguration` into a strongly typed `Options` object.
///
/// Only explicitly present keys are applied; all others retain their
/// default values.
///
/// @param[in] conf
/// Configuration object containing encoder options.
///
/// @return
/// A fully initialized `Options` structure.
///
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
    if (has<bool>(conf, "normalizeMars")) {
        opts.normalizeMars = get_or_throw<bool>(conf, "normalizeMars");
    }
    if (has<bool>(conf, "normalizeMisc")) {
        opts.normalizeMisc = get_or_throw<bool>(conf, "normalizeMisc");
    }
    if (has<bool>(conf, "fixMarsGrid")) {
        opts.fixMarsGrid = get_or_throw<bool>(conf, "fixMarsGrid");
    }
    return opts;
}

///
/// @brief Encode a value field into a GRIB message.
///
/// This function performs the complete encoding pipeline:
/// - optional metadata normalization,
/// - GRIB header construction,
/// - value injection.
///
/// The function is exception-safe and returns a fully initialized
/// `CodesHandle` owning the encoded GRIB message.
///
/// -----------------------------------------------------------------------------
/// Normalization and lifetime semantics (CRITICAL)
/// -----------------------------------------------------------------------------
///
/// Metadata normalization is **conditionally enabled** based on runtime
/// options.
///
/// The normalization step does **not** return new objects. Instead, it
/// returns **references** to the *active* metadata dictionaries:
///
/// - If normalization is **disabled**:
/// - references alias the input objects (`inputMars`, `inputMisc`)
///
/// - If normalization is **enabled**:
/// - references alias local scratch objects (`scratchMars`, `scratchMisc`)
/// - the scratch objects contain normalized copies of the inputs
///
/// The returned references must be treated as **borrowed**:
/// - they must not be stored,
/// - they must not escape this function,
/// - their lifetime is strictly limited to this scope.
///
/// This contract allows the pipeline to avoid unnecessary allocations when
/// normalization is disabled, while preserving correctness when it is enabled.
///
/// -----------------------------------------------------------------------------
/// @tparam Val_t
/// Numeric type of the values to be encoded.
///
/// @param values
/// Contiguous span of values to encode.
///
/// @param inputMars
/// Input MARS request configuration (read-only).
///
/// @param inputMisc
/// Input miscellaneous configuration (read-only).
///
/// @param options
/// Encoding options controlling behavior such as validation,
/// logging, or feature toggles.
///
/// @param language
/// MARS language definition
///
/// @return
/// A `std::unique_ptr` owning the encoded GRIB handle.
///
/// @throws mars2grib::Exception
/// If normalization, header encoding, or value encoding fails.
///
template <typename T>
std::unique_ptr<metkit::codes::CodesHandle> encode_impl(const metkit::codes::Span<const T>& values,
                                                        const eckit::LocalConfiguration& inputMars,
                                                        const eckit::LocalConfiguration& inputMisc,
                                                        const Options options, const eckit::Value& language) {

    using metkit::mars2grib::utils::exceptions::printExtendedStack;

    // 1. Prepare Scratches for Normalization
    eckit::LocalConfiguration scratchMars;
    eckit::LocalConfiguration scratchMisc;

    try {

        // 2. Normalize Metadata (conditionally)
        // -----------------------------------------------------------------
        // IMPORTANT: Normalization returns *references*, not values.
        //
        // Depending on runtime options:
        //   - normalization DISABLED  -> activeMars / activeMisc alias inputs
        //   - normalization ENABLED   -> activeMars / activeMisc alias scratch
        //
        // The returned references are BORROWED and MUST NOT escape this scope.
        // Their lifetime is bounded by `scratchMars` / `scratchMisc`.
        // -----------------------------------------------------------------
        auto [activeMars, activeMisc] =
            CoreOperations::normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars, scratchMisc);

        // 3. Encode Header (SpecializedEncoder creates the CodesHandle here)
        auto gribHeader = CoreOperations::encodeHeader<eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                                       metkit::codes::CodesHandle>(activeMars, activeMisc, options);

        // 4. Inject Values
        return CoreOperations::encodeValues(values, activeMisc, options, std::move(gribHeader));
    }
    catch (const std::exception& e) {
        printExtendedStack(e);
        throw;
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw metkit::mars2grib::utils::exceptions::Mars2GribGenericException("Unknown error during encoding", Here());
    }
}

}  // namespace


// -----------------------------------------------------------------------------
// Mars2Grib construction
// -----------------------------------------------------------------------------

Mars2Grib::Mars2Grib() : opts_{} {}

Mars2Grib::Mars2Grib(const Options& opts) : opts_{opts} {}

Mars2Grib::Mars2Grib(const eckit::LocalConfiguration& opts) : opts_{readOptions(opts)} {}


// -----------------------------------------------------------------------------
// Encoding interfaces
// -----------------------------------------------------------------------------
std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode(Span<const double>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {

    return encode(Span<const float>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(Span<const double>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(Span<const float>{values}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode(Span<const double>{values, length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode(Span<const float>{values, length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(Span<const double>{values, length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(Span<const float>{values, length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const metkit::codes::Span<const double>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode_impl(values, mars, misc, opts_, language_);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const metkit::codes::Span<const float>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode_impl(values, mars, misc, opts_, language_);
}

}  // namespace metkit::mars2grib

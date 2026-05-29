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
/// - This file is part of the **Mars2Grib public API implementation**
/// - It is not intended for direct use by end users
/// - Its behavior defines the observable semantics of `Mars2Grib::encode`
///   and the temporary staged-encoding API
///
/// @ingroup mars2grib_api
///


#include "Mars2Grib.h"

// other libraries
#include "eckit/exception/Exceptions.h"

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// encode header/values implementation
#include "metkit/mars2grib/CoreOperations.h"

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

}  // namespace

///
/// @brief Concrete specialization of the generic core cache entry.
///
/// This alias binds the generic `CoreOperations::CacheEntry` template to the
/// concrete public API types:
/// - `eckit::LocalConfiguration` for MARS metadata
/// - `eckit::LocalConfiguration` for auxiliary metadata
/// - `Options` for encoding options
/// - `metkit::codes::CodesHandle` for the GRIB output object
///
/// It is used internally as the implementation payload of the opaque
/// `Mars2Grib::CacheEntry`.
///
/// @note
/// This staged cache path is temporary and not intended for public use.
///
using CoreCacheEntry = CoreOperations::CacheEntry<eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                                  metkit::codes::CodesHandle>;

///
/// @brief Opaque API wrapper around the concrete core cache entry.
///
/// This type hides the concrete staged-encoding cache representation from
/// API users while preserving unique ownership and immutability.
///
/// Internally, it stores a fully prepared `CoreCacheEntry` specialized for
/// the public `Mars2Grib` API types.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
/// It is exposed only for transitional benchmarking and comparison
/// purposes and may be changed or removed without notice.
///
struct Mars2Grib::CacheEntry {

    ///
    /// @brief Construct the opaque wrapper from a concrete core cache.
    ///
    /// Ownership of the prepared core cache is transferred into the wrapper.
    ///
    /// @param[in] impl
    /// Unique pointer owning the prepared concrete core cache entry.
    ///
    explicit CacheEntry(std::unique_ptr<const CoreCacheEntry>&& impl) : impl_{std::move(impl)} {}

    CacheEntry(const CacheEntry&)            = delete;
    CacheEntry& operator=(const CacheEntry&) = delete;
    CacheEntry(CacheEntry&&)                 = delete;
    CacheEntry& operator=(CacheEntry&&)      = delete;
    ~CacheEntry()                            = default;

    ///
    /// @brief Owned immutable implementation cache.
    ///
    /// This pointer stores the concrete staged-encoding cache used by the
    /// generic core layer.
    ///
    const std::unique_ptr<const CoreCacheEntry> impl_;
};

///
/// @brief Destroy an opaque API cache entry.
///
/// This custom deleter is required because `Mars2Grib::CacheEntry` is
/// incomplete in the public header and only fully defined in this
/// implementation file.
///
/// @param[in] p
/// Pointer to the opaque cache entry to destroy.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
///
void Mars2Grib::CacheEntryDeleter::operator()(const CacheEntry* p) const {
    delete p;
}

///
/// @brief Prepare an opaque staged-encoding cache.
///
/// This function delegates to the generic core layer to:
/// - normalize input metadata if enabled,
/// - resolve the structural header layout,
/// - build the specialized encoder state,
/// - prepare an immutable reusable sample for later finalization.
///
/// The returned cache is opaque at the public API level and can be
/// passed to `finaliseEncoding()` multiple times.
///
/// @param[in] mars
/// MARS dictionary describing the field metadata.
///
/// @param[in] misc
/// Auxiliary metadata dictionary.
///
/// @return
/// A unique pointer owning an opaque immutable cache entry.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
/// It is exposed only for transitional benchmarking and comparison
/// purposes and may be changed or removed without notice.
///
Mars2Grib::CacheEntryPtr Mars2Grib::prepare(const eckit::LocalConfiguration& mars,
                                            const eckit::LocalConfiguration& misc) {
    auto coreCache = CoreOperations::prepare<eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                             metkit::codes::CodesHandle>(mars, misc, opts_, language_);

    return CacheEntryPtr(new CacheEntry(std::move(coreCache)));
}


///
/// @brief Finalize a GRIB encoding from a previously prepared cache.
///
/// This function delegates to the generic core layer to complete the
/// encoding pipeline using a cache previously produced by `prepare()`.
///
/// The cache provides the pre-specialized, reusable encoding state,
/// while this function injects the supplied field values and produces
/// a final `CodesHandle`.
///
/// The cache is not consumed and may be reused in subsequent calls.
///
/// @param[in] cacheEntry
/// Opaque staged-encoding cache previously produced by `prepare()`.
///
/// @param[in] values
/// Field values to encode as double.
///
/// @param[in] mars
/// MARS dictionary describing the field metadata.
///
/// @param[in] misc
/// Auxiliary metadata dictionary.
///
/// @return
/// A unique pointer to a GRIB handle containing the encoded message.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
/// It is exposed only for transitional benchmarking and comparison
/// purposes and may be changed or removed without notice.
///
std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::finaliseEncoding(const CacheEntryPtr& cacheEntry,
                                                                        const std::vector<double>& values,
                                                                        const eckit::LocalConfiguration& mars,
                                                                        const eckit::LocalConfiguration& misc) {
    return CoreOperations::finaliseEncoding<double, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                            metkit::codes::CodesHandle>(
        *(cacheEntry->impl_), Span<const double>{values}, mars, misc, opts_, language_);
}


///
/// @brief Finalize a GRIB encoding from a previously prepared cache.
///
/// This overload accepts field values as `float`.
///
/// The cache supplies the pre-specialized, reusable encoding state,
/// while this function injects the provided field values and applies
/// any remaining dynamic metadata handling required to produce a final
/// GRIB message.
///
/// The cache is not consumed by this operation and may be reused
/// for subsequent calls.
///
/// @param[in] cacheEntry
/// Opaque staged-encoding cache previously produced by `prepare()`.
///
/// @param[in] values
/// Pointer to the field values as double.
///
/// @param[in] length
/// Number of values in the buffer.
///
/// @param[in] mars
/// MARS dictionary describing the field metadata.
///
/// @param[in] misc
/// Auxiliary metadata dictionary.
///
/// @return
/// A unique pointer to a GRIB handle containing the encoded message.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
/// It is exposed only for transitional benchmarking and comparison
/// against a legacy implementation. It may be changed or removed
/// without notice.
///
std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::finaliseEncoding(const CacheEntryPtr& cacheEntry,
                                                                        const double* values, size_t length,
                                                                        const eckit::LocalConfiguration& mars,
                                                                        const eckit::LocalConfiguration& misc) {
    return CoreOperations::finaliseEncoding<double, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                            metkit::codes::CodesHandle>(
        *(cacheEntry->impl_), Span<const double>{values, length}, mars, misc, opts_, language_);
}

///
/// @brief Finalize a GRIB encoding from a previously prepared cache.
///
/// This overload accepts field values as `float`.
///
/// The cache provides the pre-specialized, reusable encoding state,
/// while this function injects the supplied field values and produces
/// a final `CodesHandle`.
///
/// The cache is not consumed and may be reused in subsequent calls.
///
/// @param[in] cacheEntry
/// Opaque staged-encoding cache previously produced by `prepare()`.
///
/// @param[in] values
/// Field values to encode as float.
///
/// @param[in] mars
/// MARS dictionary describing the field metadata.
///
/// @param[in] misc
/// Auxiliary metadata dictionary.
///
/// @return
/// A unique pointer to a GRIB handle containing the encoded message.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
/// It is exposed only for transitional benchmarking and comparison
/// purposes and may be changed or removed without notice.
///
std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::finaliseEncoding(const CacheEntryPtr& cacheEntry,
                                                                        const std::vector<float>& values,
                                                                        const eckit::LocalConfiguration& mars,
                                                                        const eckit::LocalConfiguration& misc) {
    return CoreOperations::finaliseEncoding<float, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                            metkit::codes::CodesHandle>(*(cacheEntry->impl_), Span<const float>{values},
                                                                        mars, misc, opts_, language_);
}

///
/// @brief Finalize a GRIB encoding from a previously prepared cache.
///
/// This overload accepts field values as `float`.
///
/// The cache supplies the pre-specialized, reusable encoding state,
/// while this function injects the provided field values and applies
/// any remaining dynamic metadata handling required to produce a final
/// GRIB message.
///
/// The cache is not consumed by this operation and may be reused
/// for subsequent calls.
///
/// @param[in] cacheEntry
/// Opaque staged-encoding cache previously produced by `prepare()`.
///
/// @param[in] values
/// Pointer to the field values as float.
///
/// @param[in] length
/// Number of values in the buffer.
///
/// @param[in] mars
/// MARS dictionary describing the field metadata.
///
/// @param[in] misc
/// Auxiliary metadata dictionary.
///
/// @return
/// A unique pointer to a GRIB handle containing the encoded message.
///
/// @note
/// This staged cache API is temporary and not intended for public use.
/// It is exposed only for transitional benchmarking and comparison
/// against a legacy implementation. It may be changed or removed
/// without notice.
///
std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::finaliseEncoding(const CacheEntryPtr& cacheEntry,
                                                                        const float* values, size_t length,
                                                                        const eckit::LocalConfiguration& mars,
                                                                        const eckit::LocalConfiguration& misc) {
    return CoreOperations::finaliseEncoding<float, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                            metkit::codes::CodesHandle>(
        *(cacheEntry->impl_), Span<const float>{values, length}, mars, misc, opts_, language_);
}


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
    try {
        return CoreOperations::encode<double, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const double>{values}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    try {
        return CoreOperations::encode<float, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const float>{values}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars) {
    try {
        const eckit::LocalConfiguration misc{};
        return CoreOperations::encode<double, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const double>{values}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars) {
    try {
        const eckit::LocalConfiguration misc{};
        return CoreOperations::encode<float, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const float>{values}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    try {
        return CoreOperations::encode<double, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const double>{values, length}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    try {
        return CoreOperations::encode<float, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const float>{values, length}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    try {
        const eckit::LocalConfiguration misc{};
        return CoreOperations::encode<double, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const double>{values, length}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    try {
        const eckit::LocalConfiguration misc{};
        return CoreOperations::encode<float, eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                      metkit::codes::CodesHandle>(Span<const float>{values, length}, mars, misc, opts_,
                                                                  language_);
    }
    catch (const std::exception& e) {
        std::string logFile = CoreOperations::generateStack( e, "Error during API::encode call", Here() );
        throw eckit::Exception("Error during encoding", Here());
    }
    catch (...) {
        // Fallback for non-standard exceptions
        throw eckit::Exception("Unknown error during encoding", Here());
    }
}

}  // namespace metkit::mars2grib

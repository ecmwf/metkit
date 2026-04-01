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
/// @file Mars2Grib.h
/// @brief High-level API for encoding MARS fields into GRIB messages.
///
/// This header defines the **public Mars2Grib encoding API**, providing a
/// user-facing interface to convert MARS-style metadata and field values
/// into GRIB messages.
///
/// The `Mars2Grib` class acts as a **stateless encoder facade**:
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
/// - A `Mars2Grib` instance is safe to use from a single thread.
/// - Concurrent use from multiple threads requires separate instances.
///
/// @ingroup mars2grib_api
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

// mars2grib public options
#include "metkit/mars2grib/api/Options.h"

namespace metkit::mars2grib {

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
/// The `Mars2Grib` class provides a **single-entry-point API**
/// for encoding numerical field data together with MARS metadata
/// into a GRIB message.
///
/// A `Mars2Grib` object encapsulates a fixed set of encoding options
/// and can be reused to encode multiple fields with the same
/// configuration.
///
/// ### Lifetime and ownership
///
/// - `Mars2Grib` does not own any external resources.
/// - Each call to `encode()` returns a new `CodesHandle` owned
/// by the caller.
///
/// ### Copy semantics
///
/// Copy and move operations are explicitly disabled to avoid
/// accidental sharing of internal state.
///
class Mars2Grib {
public:

    template <typename T>
    using Span = metkit::codes::Span<T>;

    ///
    /// @brief Construct a Mars2Grib encoder with default options.
    ///
    /// Default options correspond to standard mars2grib behavior.
    ///
    Mars2Grib();

    ///
    /// @brief Construct a Mars2Grib encoder with explicit options.
    ///
    /// @param[in] opts
    /// Encoding options controlling behavior such as validation,
    /// logging, or feature toggles.
    ///
    explicit Mars2Grib(const Options& opts);

    ///
    /// @brief Construct a Mars2Grib encoder from a configuration object.
    ///
    /// This constructor allows options to be provided via an
    /// `eckit::LocalConfiguration`, typically originating from YAML
    /// or JSON configuration files.
    ///
    /// @param[in] opts
    /// Configuration object describing encoder options.
    ///
    explicit Mars2Grib(const eckit::LocalConfiguration& opts);

    Mars2Grib(const Mars2Grib&)           = delete;
    Mars2Grib(Mars2Grib&&)                = delete;
    Mars2Grib operator=(const Mars2Grib&) = delete;
    Mars2Grib operator=(Mars2Grib&&)      = delete;

    ~Mars2Grib() = default;

    // ------------------------------------------------------------------
    // Encoding interface — std::vector based
    // ------------------------------------------------------------------

    ///
    /// @brief Encode a field into a GRIB message.
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
    std::unique_ptr<metkit::codes::CodesHandle> encode(const std::vector<double>& values,
                                                       const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc);

    ///
    /// @brief Encode a field into a GRIB message.
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
    std::unique_ptr<metkit::codes::CodesHandle> encode(const std::vector<float>& values,
                                                       const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc);

    ///
    /// @brief Encode a field using only a MARS dictionary.
    ///
    /// This overload omits the `misc` dictionary.
    ///
    /// @param[in] values
    /// Field values to encode as double.
    ///
    /// @param[in] mars
    /// MARS dictionary describing the field metadata.
    /// @return
    /// A unique pointer to a GRIB handle containing the encoded message.
    ///
    std::unique_ptr<metkit::codes::CodesHandle> encode(const std::vector<double>& values,
                                                       const eckit::LocalConfiguration& mars);

    ///
    /// @brief Encode a field using only a MARS dictionary.
    ///
    /// This overload omits the `misc` dictionary.
    ///
    /// @param[in] values
    /// Field values to encode as float.
    ///
    /// @param[in] mars
    /// MARS dictionary describing the field metadata.
    ///
    /// @return
    /// A unique pointer to a GRIB handle containing the encoded message.
    ///
    std::unique_ptr<metkit::codes::CodesHandle> encode(const std::vector<float>& values,
                                                       const eckit::LocalConfiguration& mars);


    // ------------------------------------------------------------------
    // Encoding interface — raw pointer based
    // ------------------------------------------------------------------

    ///
    /// @brief Encode a field from a raw value buffer.
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
    std::unique_ptr<metkit::codes::CodesHandle> encode(const double* values, size_t length,
                                                       const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc);

    ///
    /// @brief Encode a field from a raw value buffer.
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
    std::unique_ptr<metkit::codes::CodesHandle> encode(const float* values, size_t length,
                                                       const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc);

    ///
    /// @brief Encode a field from a raw value buffer.
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
    /// @return
    /// A unique pointer to a GRIB handle containing the encoded message.
    ///
    std::unique_ptr<metkit::codes::CodesHandle> encode(const double* values, size_t length,
                                                       const eckit::LocalConfiguration& mars);

    ///
    /// @brief Encode a field from a raw value buffer.
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
    /// @return
    /// A unique pointer to a GRIB handle containing the encoded message.
    ///
    std::unique_ptr<metkit::codes::CodesHandle> encode(const float* values, size_t length,
                                                       const eckit::LocalConfiguration& mars);

    ///
    /// @brief Opaque cache object for staged GRIB encoding.
    ///
    /// This type represents a precomputed encoding cache produced by
    /// `prepare()` and later consumed by `finaliseEncoding()`.
    ///
    /// The cache internalizes the expensive metadata-specialization phase
    /// and stores immutable state that can be reused across multiple
    /// finalization calls.
    ///
    /// The concrete representation of this type is intentionally hidden
    /// from API users.
    ///
    /// @note
    /// This staged cache API is temporary and not intended for public use.
    /// It is exposed only for transitional benchmarking and comparison
    /// against a legacy implementation. It may be changed or removed
    /// without notice.
    ///
    struct CacheEntry;

    ///
    /// @brief Custom deleter for opaque cache entries.
    ///
    /// This deleter is paired with `CacheEntryPtr` to allow ownership of
    /// an incomplete, opaque `CacheEntry` type in the public interface.
    ///
    /// @note
    /// This staged cache API is temporary and not intended for public use.
    /// It is exposed only for transitional benchmarking and comparison
    /// against a legacy implementation. It may be changed or removed
    /// without notice.
    ///
    struct CacheEntryDeleter {

        ///
        /// @brief Destroy an opaque cache entry.
        ///
        /// @param[in] cache
        /// Pointer to the cache entry to destroy.
        ///
        void operator()(const CacheEntry*) const;
    };

    ///
    /// @brief Owning smart pointer to an opaque staged-encoding cache.
    ///
    /// This alias represents unique ownership of a `CacheEntry`
    /// produced by `prepare()`.
    ///
    /// The pointed object is immutable and may be reused across
    /// multiple `finaliseEncoding()` calls.
    ///
    /// @note
    /// This staged cache API is temporary and not intended for public use.
    /// It is exposed only for transitional benchmarking and comparison
    /// against a legacy implementation. It may be changed or removed
    /// without notice.
    ///
    using CacheEntryPtr = std::unique_ptr<const CacheEntry, CacheEntryDeleter>;

    ///
    /// @brief Prepare a reusable staged-encoding cache.
    ///
    /// This function performs the metadata-specialization phase of the
    /// encoding pipeline and returns an opaque cache object that can be
    /// reused in subsequent calls to `finaliseEncoding()`.
    ///
    /// The cache is derived from:
    /// - the MARS dictionary,
    /// - the auxiliary metadata dictionary,
    /// - the current `Mars2Grib` options,
    /// - the active language definition.
    ///
    /// This operation is intended for workflows where the structural
    /// encoding setup is reused multiple times with different field values
    /// and/or repeated finalization requests.
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
    /// against a legacy implementation. It may be changed or removed
    /// without notice.
    ///
    CacheEntryPtr prepare(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc);

    ///
    /// @brief Finalize a GRIB encoding from a previously prepared cache.
    ///
    /// This function completes the encoding pipeline using a cache
    /// previously produced by `prepare()`.
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
    /// against a legacy implementation. It may be changed or removed
    /// without notice.
    ///
    std::unique_ptr<metkit::codes::CodesHandle> finaliseEncoding(const CacheEntryPtr& cacheEntry,
                                                                 const std::vector<double>& values,
                                                                 const eckit::LocalConfiguration& mars,
                                                                 const eckit::LocalConfiguration& misc);

    ///
    /// @brief Finalize a GRIB encoding from a previously prepared cache.
    ///
    /// This function completes the encoding pipeline using a cache
    /// previously produced by `prepare()`.
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
    std::unique_ptr<metkit::codes::CodesHandle> finaliseEncoding(const CacheEntryPtr& cacheEntry, const double* values,
                                                                 size_t length, const eckit::LocalConfiguration& mars,
                                                                 const eckit::LocalConfiguration& misc);

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
    /// against a legacy implementation. It may be changed or removed
    /// without notice.
    ///
    std::unique_ptr<metkit::codes::CodesHandle> finaliseEncoding(const CacheEntryPtr& cacheEntry,
                                                                 const std::vector<float>& values,
                                                                 const eckit::LocalConfiguration& mars,
                                                                 const eckit::LocalConfiguration& misc);


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
    std::unique_ptr<metkit::codes::CodesHandle> finaliseEncoding(const CacheEntryPtr& cacheEntry, const float* values,
                                                                 size_t length, const eckit::LocalConfiguration& mars,
                                                                 const eckit::LocalConfiguration& misc);

private:

    const eckit::Value language_;
    const Options opts_;
};

}  // namespace metkit::mars2grib

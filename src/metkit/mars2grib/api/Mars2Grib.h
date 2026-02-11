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
#include <tuple>
#include <utility>
#include <vector>

// eckit
#include "eckit/config/LocalConfiguration.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"

// ecCodes API wrapper
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Codes wrapper types
#include "metkit/codes/api/CodesTypes.h"

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// mars2grib public options
#include "metkit/mars2grib/CoreOperations.h"
#include "metkit/mars2grib/api/Options.h"

namespace metkit::mars2grib {

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

private:

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
    /// @return
    /// A `std::unique_ptr` owning the encoded GRIB handle.
    ///
    /// @throws mars2grib::Exception
    /// If normalization, header encoding, or value encoding fails.
    ///
    template <typename Val_t>
    std::unique_ptr<metkit::codes::CodesHandle> encode_impl(const Span<const Val_t>& values,
                                                            const eckit::LocalConfiguration& inputMars,
                                                            const eckit::LocalConfiguration& inputMisc) {

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
                CoreOperations::normalize_if_enabled(inputMars, inputMisc, opts_, language_, scratchMars, scratchMisc);

            // 3. Encode Header (SpecializedEncoder creates the CodesHandle here)
            auto gribHeader =
                CoreOperations::encodeHeader<eckit::LocalConfiguration, eckit::LocalConfiguration, Options,
                                             metkit::codes::CodesHandle>(activeMars, activeMisc, opts_);

            // 4. Inject Values
            return CoreOperations::encodeValues(values, activeMisc, opts_, std::move(gribHeader));
        }
        catch (const std::exception& e) {
            printExtendedStack(e);
            throw;
        }
        catch (...) {
            // Fallback for non-standard exceptions
            throw metkit::mars2grib::utils::exceptions::Mars2GribGenericException("Unknown error during encoding",
                                                                                  Here());
        }
    }

    const eckit::Value language_;
    const Options opts_;
};

}  // namespace metkit::mars2grib

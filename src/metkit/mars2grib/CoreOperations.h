/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file CoreOperations.h
/// @brief High-level service layer for GRIB encoding and resolution orchestration.
///
/// This header defines the `CoreOperations` suite, providing the primary
/// functional building blocks for the mars2grib library.
///
/// These operations facilitate a staged translation pipeline:
/// 1. **Sanitization**: Normalizing input dictionaries against the language definition.
/// 2. **Header Resolution**: Determining the GRIB structural layout and encoding metadata.
/// 3. **Value Injection**: Physical realization of the GRIB data section.
/// 4. **Diagnostic Capture**: Generating regression data for structural validation.
///
///
/// ---
///
/// ## Transitional cache preparation
///
/// This header also contains a temporary staged-encoding path based on
/// `prepare()` and `finaliseEncoding()`.
///
/// This code is a preparatory step toward a future implementation where
/// cache lifecycle and reuse are fully internalized inside the core layer.
///
/// At the current stage, the staged API exists to support benchmarking,
/// comparison, and incremental integration of the future cache design.
///
/// @ingroup mars2grib_core
///
#pragma once

// System includes
#include <string>
#include <tuple>
#include <utility>
#include <cstdio>
#include <unistd.h>

#if defined(__APPLE__) && defined(__MACH__)
    #include <pthread.h>
#elif defined(__linux__)
    #include <sys/syscall.h>
#endif


/// Project includes
/// @note: clang-format needs to be off here to preserve the logical grouping of
/// includes and avoid unnecessary reordering that breaks the layering and dependencies.
// clang-format off
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/encodeValues.h"
#include "metkit/mars2grib/frontend/header/SpecializedEncoder.h"
#include "metkit/mars2grib/frontend/make_HeaderLayout.h"
#include "metkit/mars2grib/frontend/normalization/normalization.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
// clang-format on

namespace metkit::mars2grib {


namespace detail {

/// @brief Generates a strictly formatted log filename safely (Linux/macOS only).
/// Guarantees no exceptions will escape and crash the simulation.
inline std::string generateEncodingLogFilename() noexcept {
    try {
        // 1. Get true Process ID
        pid_t pid = getpid();

        // 2. Get true OS Thread ID
        unsigned long long tid = 0;

#if defined(__APPLE__) && defined(__MACH__)
        uint64_t mac_tid;
        pthread_threadid_np(NULL, &mac_tid);
        tid = static_cast<unsigned long long>(mac_tid);
#elif defined(__linux__)
        // SYS_gettid is universally supported on Linux, even on old glibc versions
        tid = static_cast<unsigned long long>(syscall(SYS_gettid));
#else
        // Ultimate fallback if compiled on an unknown UNIX variant
        tid = 0;
#endif

        // 3. Format the string: encodingStack_%06d_%06llu.txt
        // %06d ensures at least 6 digits with leading zeros (equivalent to %6.6d)
        char buffer[64];
        int written = std::snprintf(buffer, sizeof(buffer),
                                    "encodingStack_%06d_%06llu.txt",
                                    static_cast<int>(pid), tid);

        if (written > 0 && static_cast<size_t>(written) < sizeof(buffer)) {
            return std::string(buffer);
        }

        return "encodingStack_fallback.txt";

    } catch (...) {
        // Catches std::bad_alloc if std::string creation fails due to OOM
        return "encodingStack_fallback.txt";
    }
}

} // namespace detail


///
/// @brief Internal engine providing atomic encoding and diagnostic services.
///
struct CoreOperations {

    ///
    /// @brief Normalize input dictionaries against the library language definition.
    ///
    /// This operation performs key-value sanitization for both MARS and Parameter
    /// metadata. It utilizes a **reference-redirection strategy**: if no
    /// modification is required, the returned references point to the original
    /// inputs; otherwise, they point to the provided scratch buffers.
    ///
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam ParDict_t  Parameter dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    ///
    /// @param[in]  inputMars   Original MARS request
    /// @param[in]  inputMisc   Original Parameter metadata
    /// @param[in]  opt         Encoding options
    /// @param[in]  lang        Language definition (eckit::Value)
    /// @param[out] scratchMars Scratch buffer for MARS sanitization
    /// @param[out] scratchMisc Scratch buffer for Parameter sanitization
    ///
    /// @return A tuple containing const references to the active (sanitized) data
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t>
    static std::tuple<const MarsDict_t&, const ParDict_t&> normalize_if_enabled(
        const MarsDict_t& inputMars, const ParDict_t& inputMisc, const OptDict_t& opt, const eckit::Value& lang,
        MarsDict_t& scratchMars, ParDict_t& scratchMisc) {

        try {
            const MarsDict_t& activeMars =
                frontend::normalization::normalize_MarsDict_if_enabled(inputMars, opt, lang, scratchMars);
            const ParDict_t& activePar =
                frontend::normalization::normalize_MiscDict_if_enabled(inputMisc, opt, lang, scratchMisc);

            return {activeMars, activePar};
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribCoreException("Error during metadata normalization", Here()));
        }
    }

    ///
    /// @brief Resolve and encode GRIB header metadata.
    ///
    /// Executes the structural resolution phase to determine the GRIB layout
    /// and triggers the specialized metadata encoder to populate the header
    /// sections of the output object.
    ///
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam ParDict_t  Parameter dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    /// @tparam OutDict_t  Output GRIB handle/dictionary type
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<OutDict_t> encodeHeader(const MarsDict_t& mars, const ParDict_t& misc,
                                                   const OptDict_t& opt) {

        try {
            using metkit::mars2grib::frontend::make_HeaderLayout_or_throw;
            using metkit::mars2grib::frontend::header::SpecializedEncoder;

            auto layout = make_HeaderLayout_or_throw(mars, opt);

            return SpecializedEncoder<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>{std::move(layout)}.encode(mars, misc,
                                                                                                             opt);
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribCoreException("Error during header encoding", Here()));
        }
    }


    ///
    /// @brief Inject numeric field values into a GRIB handle.
    ///
    /// A procedural operation that handles bitmap generation and physical
    /// data compression. Utilizes spans for zero-copy data passing.
    ///
    /// @tparam Val_t      Numeric precision (float or double)
    /// @tparam ParDict_t  Parameter dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    /// @tparam OutDict_t  Output GRIB handle/dictionary type
    ///
    template <typename Val_t, class ParDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<OutDict_t> encodeValues(backend::Span<const Val_t> values, const ParDict_t& misc,
                                                   const OptDict_t& opt, std::unique_ptr<OutDict_t> handle) {

        try {
            metkit::mars2grib::backend::encodeValues(values, misc, opt, *handle);
            return handle;
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribCoreException("Error during value encoding", Here()));
        }
    }

    ///
    /// @brief Encode a value field into a GRIB message.
    ///
    /// This function performs the complete encoding pipeline:
    /// - optional metadata normalization,
    /// - GRIB header construction,
    /// - value injection.
    ///
    /// The function is exception-safe and returns an output dictionary. This can be a fully initialized `CodesHandle`
    /// owning the encoded GRIB message.
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
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam ParDict_t Parameterization dictionary type
    /// @tparam OptDict_t Options dictionary type
    /// @tparam OutDict_t Output dictionary type
    ///
    /// @param values
    /// Contiguous span of values to encode.
    ///
    /// @param inputMars
    /// Input MARS description of the data (read-only).
    ///
    /// @param inputMisc
    /// Input miscellaneous description of the data (read-only).
    ///
    /// @param options
    /// Encoding options controlling behavior such as validation,
    /// logging, or feature toggles.
    ///
    /// @param language
    /// MARS language definition
    ///
    /// @return
    /// A `std::unique_ptr` with GRIB keys set
    ///
    /// @throws mars2grib::Exception
    /// If normalization, header encoding, or value encoding fails.
    ///
    template <typename Val_t, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<OutDict_t> encode(const metkit::codes::Span<const Val_t>& values,
                                             const MarsDict_t& inputMars, const ParDict_t& inputMisc,
                                             const OptDict_t& options, const eckit::Value& language) {

        using metkit::mars2grib::utils::exceptions::printExtendedStack;

        // 1. Prepare Scratches for Normalization
        MarsDict_t scratchMars;
        ParDict_t scratchMisc;

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
                normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars, scratchMisc);

            // 3. Encode Header (SpecializedEncoder creates the CodesHandle here)
            auto gribHeader =
                encodeHeader<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(activeMars, activeMisc, options);

            // 4. Inject Values
            return encodeValues(values, activeMisc, options, std::move(gribHeader));
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribCoreException("Error during encoding", Here()));
        }
    }

    // -------------------------------------------------------------------------
    // Temporary staged-cache preparation path
    // -------------------------------------------------------------------------
    //
    // The following types and functions implement a transitional staged
    // preparation/finalisation workflow.
    //
    // This code exists as a preparatory step toward a future version where
    // cache construction, reuse, and lifecycle are fully internalized in the
    // core encoding layer.
    //

    ///
    /// @brief Immutable staged-encoding cache entry.
    ///
    /// This object stores the reusable state required to split the encoding
    /// pipeline into two phases:
    ///
    /// 1. **Preparation**
    ///    - metadata normalization
    ///    - header layout resolution
    ///    - encoder specialization
    ///    - creation of a reusable prepared sample
    ///
    /// 2. **Finalisation**
    ///    - reuse of the prepared structural state
    ///    - injection of dynamic metadata if needed
    ///    - value encoding into a fresh output object
    ///
    /// The cache entry is intentionally immutable:
    /// - copy is disabled
    /// - move is disabled
    /// - the prepared sample is owned through an immutable smart pointer
    ///
    /// This guarantees that once constructed, the cache entry represents a
    /// stable reusable encoding context.
    ///
    /// @note
    /// This staged cache path is a preparatory step toward a future
    /// implementation where cache management is fully internalized in the
    /// core encoding layer.
    ///
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam ParDict_t  Parameter/misc dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    /// @tparam OutDict_t  Output GRIB handle/dictionary type
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
    struct CacheEntry {

        using Encoder =
            metkit::mars2grib::frontend::header::SpecializedEncoder<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
        using Layout = metkit::mars2grib::frontend::GribHeaderLayoutData;

        ///
        /// @brief Construct a cache entry from resolved layout and active metadata.
        ///
        /// This constructor:
        /// - internalizes the resolved header layout into a specialized encoder
        /// - prepares an immutable reusable sample from the active metadata
        ///
        /// The input metadata is expected to already represent the active
        /// dictionaries selected after any optional normalization step.
        ///
        /// @param[in] layout
        /// Resolved GRIB header layout to internalize.
        ///
        /// @param[in] inputMars
        /// Active MARS metadata dictionary.
        ///
        /// @param[in] inputMisc
        /// Active parameter/misc metadata dictionary.
        ///
        /// @param[in] options
        /// Encoding options controlling specialization behavior.
        ///
        /// @note
        /// This constructor is part of the temporary staged-cache path and
        /// prepares the structure required for a future internal cache design.
        ///
        CacheEntry(Layout&& layout, const MarsDict_t& inputMars, const ParDict_t& inputMisc, const OptDict_t& options) :
            encoder_{std::move(layout)}, preparedSample_{encoder_.prepare(inputMars, inputMisc, options)} {};

        CacheEntry(const CacheEntry&)            = delete;
        CacheEntry& operator=(const CacheEntry&) = delete;
        CacheEntry(CacheEntry&&)                 = delete;
        CacheEntry& operator=(CacheEntry&&)      = delete;
        ~CacheEntry()                            = default;

        ///
        /// @brief Fully specialized immutable encoder.
        ///
        /// This encoder owns the resolved layout and the precomputed execution
        /// plan used during staged finalisation.
        ///
        const Encoder encoder_;

        ///
        /// @brief Immutable prepared sample used as finalisation template.
        ///
        /// This object represents the reusable structural sample produced
        /// during the preparation phase. It is treated as read-only input
        /// during `finaliseEncoding()`, which derives a fresh output handle
        /// from it.
        ///
        const std::unique_ptr<const OutDict_t> preparedSample_;
    };

    ///
    /// @brief Prepare a reusable staged-encoding cache.
    ///
    /// This function performs the preparation phase of the staged encoding
    /// pipeline:
    /// - optional metadata normalization
    /// - header layout resolution
    /// - construction of a specialized encoder
    /// - creation of an immutable reusable sample
    ///
    /// The returned cache entry can later be passed to
    /// `finaliseEncoding()` to complete the encoding with concrete field
    /// values.
    ///
    /// -----------------------------------------------------------------------------
    /// Normalization and lifetime semantics (CRITICAL)
    /// -----------------------------------------------------------------------------
    ///
    /// As in `encode()`, normalization returns borrowed references to the
    /// active metadata dictionaries:
    /// - either the original inputs
    /// - or local scratch objects containing normalized copies
    ///
    /// These references must not escape this function except through
    /// immediate materialization into owned cache state.
    ///
    /// The resulting `CacheEntry` stores only owned immutable state and
    /// does not retain references to the scratch buffers.
    ///
    /// -----------------------------------------------------------------------------
    ///
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam ParDict_t  Parameter/misc dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    /// @tparam OutDict_t  Output GRIB handle/dictionary type
    ///
    /// @param[in] inputMars
    /// Input MARS description of the data.
    ///
    /// @param[in] inputMisc
    /// Input miscellaneous description of the data.
    ///
    /// @param[in] options
    /// Encoding options controlling specialization and normalization.
    ///
    /// @param[in] language
    /// MARS language definition.
    ///
    /// @return
    /// A unique pointer owning an immutable staged cache entry.
    ///
    /// @throws mars2grib::Exception
    /// If normalization, layout resolution, or sample preparation fails.
    ///
    /// @note
    /// This staged cache path is a preparatory step toward a future
    /// implementation where cache lifecycle and reuse are fully
    /// internalized inside the core encoding layer.
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<const CacheEntry<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>> prepare(
        const MarsDict_t& inputMars, const ParDict_t& inputMisc, const OptDict_t& options,
        const eckit::Value& language) {

        // 1. Prepare Scratches for Normalization
        MarsDict_t scratchMars;
        ParDict_t scratchMisc;

        try {
            using metkit::mars2grib::frontend::make_HeaderLayout_or_throw;

            auto [activeMars, activeMisc] =
                normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars, scratchMisc);

            auto layout = make_HeaderLayout_or_throw(activeMars, options);

            return std::make_unique<const CacheEntry<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>>(
                std::move(layout), activeMars, activeMisc, options);
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribGenericException("Error during cache preparation", Here()));
        }
    }

    ///
    /// @brief Complete an encoding from a previously prepared cache entry.
    ///
    /// This function performs the finalisation phase of the staged encoding
    /// pipeline:
    /// - optional metadata normalization
    /// - reuse of the immutable specialized encoder
    /// - reuse of the immutable prepared sample
    /// - creation of a fresh header object
    /// - injection of the field values into the resulting output handle
    ///
    /// The provided cache entry is treated as read-only and is not consumed.
    /// It may therefore be reused across multiple finalisation calls.
    ///
    /// -----------------------------------------------------------------------------
    /// Normalization and lifetime semantics (CRITICAL)
    /// -----------------------------------------------------------------------------
    ///
    /// As in `encode()` and `prepare()`, normalization yields borrowed
    /// references to the active metadata dictionaries. These references are
    /// used only within this function and are not stored.
    ///
    /// The cache entry itself is fully owned and immutable, and no references
    /// to local scratch objects escape the function.
    ///
    /// -----------------------------------------------------------------------------
    ///
    /// @tparam Val_t      Numeric type of the values to be encoded
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam ParDict_t  Parameter/misc dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    /// @tparam OutDict_t  Output GRIB handle/dictionary type
    ///
    /// @param[in] cacheEntry
    /// Immutable staged cache entry previously produced by `prepare()`.
    ///
    /// @param[in] values
    /// Contiguous span of values to encode.
    ///
    /// @param[in] inputMars
    /// Input MARS description of the data.
    ///
    /// @param[in] inputMisc
    /// Input miscellaneous description of the data.
    ///
    /// @param[in] options
    /// Encoding options controlling behavior such as validation,
    /// logging, or feature toggles.
    ///
    /// @param[in] language
    /// MARS language definition.
    ///
    /// @return
    /// A `std::unique_ptr` owning the fully encoded output object.
    ///
    /// @throws mars2grib::Exception
    /// If normalization, staged header finalisation, or value encoding fails.
    ///
    /// @note
    /// This staged cache path is a preparatory step toward a future
    /// implementation where cache lifecycle and reuse are fully
    /// internalized inside the core encoding layer.
    ///
    template <typename Val_t, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<OutDict_t> finaliseEncoding(
        const CacheEntry<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>& cacheEntry,
        const metkit::codes::Span<const Val_t>& values, const MarsDict_t& inputMars, const ParDict_t& inputMisc,
        const OptDict_t& options, const eckit::Value& language) {

        // 1. Prepare Scratches for Normalization
        MarsDict_t scratchMars;
        ParDict_t scratchMisc;

        try {

            auto [activeMars, activeMisc] =
                normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars, scratchMisc);

            auto gribHeader =
                cacheEntry.encoder_.finaliseEncoding(*(cacheEntry.preparedSample_), activeMars, activeMisc, options);

            return encodeValues(values, activeMisc, options, std::move(gribHeader));
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribGenericException("Error during encoding finalisation", Here()));
        }
    }


    ///
    /// @brief Capture a structural test point for regression analysis.
    ///
    /// Serializes the current resolution state (GRIB Blueprint)
    /// into a JSON format suitable for external validation tools.
    ///
    /// @tparam MarsDict_t MARS dictionary type
    /// @tparam OptDict_t  Encoding options dictionary type
    ///
    template <class MarsDict_t, class OptDict_t>
    static std::string dumpHeaderTest(const MarsDict_t& mars, const OptDict_t& opt) {
        try {
            using metkit::mars2grib::frontend::make_HeaderLayout_or_throw;
            using metkit::mars2grib::frontend::debug::debug_convert_GribHeaderLayoutData_to_json;

            auto layout = make_HeaderLayout_or_throw(mars, opt);

            return debug_convert_GribHeaderLayoutData_to_json(layout);
        }
        catch (...) {
            // Wrap any exception in a CoreOperations-specific exception to provide context
            std::throw_with_nested(
                mars2grib::utils::exceptions::Mars2GribCoreException("Error during header test dump", Here()));
        }
    }

    static std::string generateStack( const std::exception& e, const std::string& msg, const eckit::CodeLocation& loc ){

        using metkit::mars2grib::utils::exceptions::printExtendedStack;
        using metkit::mars2grib::utils::exceptions::lineSize;

        const std::string logName = detail::generateEncodingLogFilename();

        std::ofstream out(logName);
        std::size_t level = 0;
        std::size_t frame = 1;

        out << "+ " << std::string(lineSize, '=') << std::endl
            << "+ frame " << frame << std::endl
            << "+ " << std::string(lineSize, '-') << std::endl;

        out << "+ file:     " << loc.file() << "\n"
                              << "+ function: " << loc.func() << "\n"
                              << "+ line:     " << loc.line() << "\n"
                              << "+ link:     " << loc.file() << ":" << loc.line() << "\n"
                              << "+ message:  " << msg << "\n";

        out << "+ " << std::string(lineSize, '+') << std::endl;

        printExtendedStack( e, out, ++level, ++frame );

        return "test.log";

    }

};

}  // namespace metkit::mars2grib

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
/// @file callbacksRegistry_common.h
/// @brief Canonical compile-time vocabulary shared by all concept dispatch registries.
///
/// This header defines the **fundamental compile-time building blocks** used
/// uniformly across the mars2grib dispatch infrastructure.
///
/// Specifically, it provides:
/// - fixed pipeline dimensions (`NUM_STAGES`, `NUM_SECTIONS`),
/// - canonical numeric identifiers for encoding stages and GRIB sections,
/// - sentinel values representing non-semantic or invalid states,
/// - canonical function pointer types (`Fn`, `Fm`) for dispatch interfaces,
/// - minimal compile-time containers (`ValueList`, `TypeList`) used to model
/// value and type universes.
///
/// This header is intentionally **purely declarative**:
/// - it contains no registry construction logic,
/// - no compile-time recursion or algorithms,
/// - and no concept-specific specialization.
///
/// All higher-level registry engines rely on the definitions in this file
/// as a **stable, shared vocabulary**.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once
#include "metkit/mars2grib/utils/generalUtils.h"

// System includes
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>


namespace metkit::mars2grib::backend::compile_time_registry_engine {


///
/// @name Sentinel values
/// @{
///
/// These constants define **sentinel values** used throughout the mars2grib
/// codebase to represent special, non-semantic states.
///
/// All three identifiers intentionally share the same numeric value.
/// Their semantic distinction is purely **contextual**, improving readability
/// and intent at the call site while preserving trivial comparability.
///
/// They are defined as `inline constexpr` values rather than enums to:
/// - allow direct use in compile-time contexts,
/// - avoid implicit conversions or scoped enum verbosity,
/// - support use as array indices or placeholders.
///
/// @note
/// These values must remain outside the valid range of any real index,
/// identifier, or enumerated domain they are compared against.
///
inline constexpr std::size_t MISSING        = 999997;
inline constexpr std::size_t INVALID        = 999998;
inline constexpr std::size_t NOT_APPLICABLE = 999999;
/// @}


///
/// @name Encoding pipeline dimensions
/// @{
///
/// These constants define the fixed dimensions of the mars2grib encoding pipeline.
///
/// They are deliberately defined as `inline constexpr` values rather than enums,
/// because they are iterated over at compile time using index sequences.
///
/// Changing these values directly affects:
/// - the size of all generated dispatch tables,
/// - the number of compile-time instantiations.
///
inline constexpr std::size_t NUM_STAGES   = 4;
inline constexpr std::size_t NUM_SECTIONS = 6;
/// @}


///
/// @name Encoding stages
/// @{
///
/// Logical stages of the encoding pipeline.
///
/// Each concept may participate in zero or more stages, as determined by its
/// compile-time applicability predicate (encoded by returning nullptr in
/// concept `encodingCallbacks()` entries).
///
inline constexpr std::size_t StageAllocate = 0;  ///< Structure allocation stage
inline constexpr std::size_t StagePreset   = 1;  ///< Metadata preset stage
inline constexpr std::size_t StageOverride = 2;  ///< Metadata override stage
inline constexpr std::size_t StageRuntime  = 3;  ///< Runtime-dependent encoding
/// @}


///
/// @name GRIB2 sections
/// @{
///
/// Numeric identifiers for GRIB2 sections, aligned with the GRIB2 specification:
/// https://codes.ecmwf.int/grib/format/grib2/sections/
///
/// These values are used as compile-time indices into dispatch tables.
///
inline constexpr std::size_t SecIndicatorSection          = 0;
inline constexpr std::size_t SecIdentificationSection     = 1;
inline constexpr std::size_t SecLocalUseSection           = 2;
inline constexpr std::size_t SecGridDefinitionSection     = 3;
inline constexpr std::size_t SecProductDefinitionSection  = 4;
inline constexpr std::size_t SecDataRepresentationSection = 5;
/// @}


///
/// @brief Canonical function pointer type for concept encoding operations.
///
/// Each dispatch table cell contains a pointer to a fully specialized concept operation
/// for a fixed (stage, section, variant). Inapplicable combinations return nullptr.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using Fn = void (*)(const MarsDict_t&, const ParDict_t&, const OptDict_t&, OutDict_t&);

///
/// @brief Canonical function pointer type for concept matcher operations.
///
/// Matchers are dictionary-specialized (no stage/section/variant dimensions).
///
template <class MarsDict_t, class OptDict_t>
using Fm = std::size_t (*)(const MarsDict_t&, const OptDict_t&);


///
/// @brief Compile-time list of values.
///
/// A lightweight container for a pack of compile-time constants (typically enum values).
/// Iteration is performed via template pack expansion (no runtime loops).
///
template <auto... Vals>
struct ValueList {
    static constexpr std::size_t size = sizeof...(Vals);
};

///
/// @brief Compile-time list of types.
///
/// A lightweight container for a pack of types. Used to represent the concept universe.
///
template <typename... Ts>
struct TypeList {
    static constexpr std::size_t size = sizeof...(Ts);
};

}  // namespace metkit::mars2grib::backend::compile_time_registry_engine

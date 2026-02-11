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
/// @file EncodingCallbacksRegistry.h
/// @brief Compile-time registry of phase-level encoding callbacks.
///
/// This header defines the **fully materialized encoding dispatch table**
/// used by the mars2grib backend to perform GRIB header encoding.
///
/// -----------------------------------------------------------------------------
/// Conceptual model
/// -----------------------------------------------------------------------------
///
/// The encoding process is modeled as a three-dimensional dispatch space:
///
/// \code
/// encodingCallbacks[globalVariant][stage][section] -> Fn | nullptr
/// \endcode
///
/// where:
/// - `globalVariant` is the flattened variant index defined by `GeneralRegistry`,
/// - `stage` is a logical encoding phase (allocation, preset, override, runtime),
/// - `section` is a GRIB2 section identifier.
///
/// Each cell contains either:
/// - a fully specialized encoding function (`Fn`), or
/// - `nullptr`, indicating that the combination is not applicable.
///
/// -----------------------------------------------------------------------------
/// Capability selection
/// -----------------------------------------------------------------------------
///
/// This registry is parameterized by a compile-time **capability index**
/// (here fixed to `0`).
///
/// Capabilities allow the same concept universe to expose multiple independent
/// dispatch planes (e.g. encoding, matching, validation) without duplicating
/// registry machinery.
///
/// -----------------------------------------------------------------------------
/// Scope and responsibility
/// -----------------------------------------------------------------------------
///
/// This file is responsible only for:
/// - binding the generic phase-callback registry machinery
/// (`makePhaseCallbacksRegistry`) to:
/// - the complete concept universe (`AllConcepts`),
/// - a specific capability index,
/// - concrete dictionary types.
///
/// It does **not**:
/// - define any concept descriptors,
/// - implement any encoding logic,
/// - perform any runtime dispatch,
/// - introduce new compile-time algorithms.
///
/// -----------------------------------------------------------------------------
/// Architectural role
/// -----------------------------------------------------------------------------
///
/// `EncodingCallbacksRegistry` is the **lowest-level executable view**
/// of the compile-time registry engine.
///
/// Higher-level systems (e.g. encoding plan construction and hot-path execution)
/// consume this registry as immutable, constexpr data.
///
/// -----------------------------------------------------------------------------
/// Design constraints
/// -----------------------------------------------------------------------------
///
/// - Header-only
/// - Fully constexpr
/// - No runtime state
/// - No dynamic allocation
///
/// This header is safe to include in performance-critical translation units.
///
#pragma once

// System includes
#include <cstdint>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/makePhaseCallbacksRegistry.h"
#include "metkit/mars2grib/backend/concepts/AllConcepts.h"
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time registry exposing encoding callbacks for all concepts.
///
/// This class template materializes a complete, three-dimensional dispatch
/// table for encoding operations, specialized for a fixed set of dictionary
/// types.
///
/// @tparam MarsDict_t
/// Type of the MARS request dictionary.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary.
///
/// @tparam OptDict_t
/// Type of the encoder options dictionary.
///
/// @tparam OutDict_t
/// Type of the output GRIB handle or dictionary.
///
/// -----------------------------------------------------------------------------
/// Lifetime and usage
/// -----------------------------------------------------------------------------
///
/// All members of this registry are:
/// - `static`,
/// - `constexpr`,
/// - immutable.
///
/// No instances of this class are ever constructed.
///
/// The registry is intended to be accessed as:
///
/// \code
/// EncodingCallbacksRegistry<...>::encodingCallbacks
/// \endcode
///
/// -----------------------------------------------------------------------------
/// Dependency guarantees
/// -----------------------------------------------------------------------------
///
/// The structure and ordering of the registry are entirely determined by:
/// - `detail::AllConcepts`,
/// - the ordering of variants within each concept,
/// - the canonical pipeline dimensions (`NUM_STAGES`, `NUM_SECTIONS`).
///
/// Any change to these inputs results in a structurally different dispatch
/// table and must be treated as a breaking change.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
struct EncodingCallbacksRegistry {

    ///
    /// @brief Size of the registry along the variant dimension.
    ///
    /// This corresponds to the total number of flattened concept variants
    /// defined by `GeneralRegistry`.
    ///
    /// It defines the first (outermost) dimension of the encoding callback
    /// dispatch table.
    ///
    static constexpr std::size_t registry_size_along_dim0 = GeneralRegistry::NVariants;

    ///
    /// @brief Size of the registry along the encoding stage dimension.
    ///
    /// This corresponds to the number of logical encoding stages
    /// (e.g. allocation, preset, override, runtime) defined by
    /// `GeneralRegistry`.
    ///
    /// It defines the second dimension of the encoding callback
    /// dispatch table.
    ///
    static constexpr std::size_t registry_size_along_dim1 = GeneralRegistry::NStages;

    ///
    /// @brief Size of the registry along the GRIB section dimension.
    ///
    /// This corresponds to the number of GRIB sections handled by the
    /// encoding pipeline, as defined by `GeneralRegistry`.
    ///
    /// It defines the third (innermost) dimension of the encoding callback
    /// dispatch table.
    ///
    static constexpr std::size_t registry_size_along_dim2 = GeneralRegistry::NSections;


    ///
    /// @brief Canonical encoding function pointer type.
    ///
    /// This alias exposes the exact function signature used for all encoding
    /// callbacks stored in the registry.
    ///
    /// It is provided primarily for:
    /// - readability,
    /// - consistency with higher-level abstractions,
    /// - avoiding repetition of long qualified names.
    ///
    using Fn_t =
        metkit::mars2grib::backend::compile_time_registry_engine::Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;

    ///
    /// @brief Fully materialized encoding dispatch table.
    ///
    /// This static data member contains the complete phase-level encoding
    /// callback registry for:
    /// - all concepts,
    /// - all variants,
    /// - all encoding stages,
    /// - all GRIB sections.
    ///
    /// The table is generated entirely at compile time by invoking
    /// `makePhaseCallbacksRegistry` with:
    /// - the full concept universe (`AllConcepts`),
    /// - capability index `0`,
    /// - the dictionary types bound to this registry.
    ///
    /// Each entry is either:
    /// - a valid function pointer, or
    /// - `nullptr` if the concept/variant/stage/section combination
    /// is not applicable.
    ///
    /// This table is intended to be consumed by higher-level planning
    /// and execution layers, not accessed directly by application code.
    ///
    static constexpr auto encodingCallbacks =
        metkit::mars2grib::backend::compile_time_registry_engine::makePhaseCallbacksRegistry<
            detail::AllConcepts, 0, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>();


    ///
    /// @brief Compile-time structural verification.
    ///
    /// These static assertions ensure that the dimensions of the generated
    /// encoding callback table exactly match the canonical sizes defined by
    /// `GeneralRegistry`.
    ///
    /// Any mismatch indicates a structural inconsistency between the
    /// registry engine and the concept universe.
    ///
    static_assert(encodingCallbacks.size() == registry_size_along_dim0,
                  "EncodingCallbacksRegistry: size along dimension 0 does not match GeneralRegistry");
    static_assert(encodingCallbacks[0].size() == registry_size_along_dim1,
                  "EncodingCallbacksRegistry: size along dimension 1 does not match GeneralRegistry");
    static_assert(encodingCallbacks[0][0].size() == registry_size_along_dim2,
                  "EncodingCallbacksRegistry: size along dimension 2 does not match GeneralRegistry");
};

}  // namespace metkit::mars2grib::backend::concepts_
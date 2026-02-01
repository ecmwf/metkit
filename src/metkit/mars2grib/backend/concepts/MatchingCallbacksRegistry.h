/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file MatchingCallbacksRegistry.h
 * @brief Compile-time registry of entry-level matching callbacks.
 *
 * This header defines the **fully materialized matching dispatch table**
 * used by the mars2grib backend to perform *semantic matching* of input
 * dictionaries against the concept universe.
 *
 * -----------------------------------------------------------------------------
 * Conceptual model
 * -----------------------------------------------------------------------------
 *
 * Matching is modeled as a one-dimensional dispatch space:
 *
 * \code
 * matchingCallbacks[conceptId] -> Fm | nullptr
 * \endcode
 *
 * where:
 * - `conceptId` is the compile-time concept identifier defined by
 *   `GeneralRegistry` / `EntryVariantRegistry`,
 * - `Fm` is a dictionary-specialized matcher function,
 * - `nullptr` indicates that the concept does not participate in matching
 *   for the selected capability.
 *
 * Each matcher is responsible for determining whether its corresponding
 * concept is *active* for a given pair of input dictionaries.
 *
 * -----------------------------------------------------------------------------
 * Capability selection
 * -----------------------------------------------------------------------------
 *
 * This registry is parameterized by a compile-time **capability index**
 * (here fixed to `0`).
 *
 * Capabilities allow the same concept universe to expose multiple independent
 * matching planes (e.g. alternative semantics, validation modes, feature
 * subsets) without duplicating registry infrastructure.
 *
 * -----------------------------------------------------------------------------
 * Scope and responsibility
 * -----------------------------------------------------------------------------
 *
 * This file is responsible only for:
 * - binding the generic entry-level registry machinery
 *   (`makeEntryCallbacksRegistry`) to:
 *     - the complete concept universe (`AllConcepts`),
 *     - a specific capability index,
 *     - concrete dictionary types.
 *
 * It does **not**:
 * - define any concept descriptors,
 * - implement matching logic,
 * - perform runtime dispatch or iteration,
 * - introduce new compile-time algorithms.
 *
 * -----------------------------------------------------------------------------
 * Architectural role
 * -----------------------------------------------------------------------------
 *
 * `MatchingCallbacksRegistry` provides the **semantic entry point** to the
 * resolution pipeline.
 *
 * Its output is typically consumed by:
 * - active concept resolution,
 * - semantic filtering of variants,
 * - downstream structural resolution stages.
 *
 * -----------------------------------------------------------------------------
 * Design constraints
 * -----------------------------------------------------------------------------
 *
 * - Header-only
 * - Fully constexpr
 * - No runtime state
 * - No dynamic allocation
 *
 * This header is safe to include transitively in performance-critical code.
 */
#pragma once

// System includes
#include <cstdint>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/makeEntryCallbacksRegistry.h"
#include "metkit/mars2grib/backend/concepts/AllConcepts.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time registry exposing entry-level matching callbacks.
 *
 * This class template materializes a complete table of matcher functions,
 * one per concept, specialized for a fixed pair of dictionary types.
 *
 * @tparam MarsDict_t
 *   Type of the MARS request dictionary.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary.
 *
 * -----------------------------------------------------------------------------
 * Lifetime and usage
 * -----------------------------------------------------------------------------
 *
 * All members of this registry are:
 * - `static`,
 * - `constexpr`,
 * - immutable.
 *
 * No instances of this class are ever constructed.
 *
 * The registry is intended to be accessed as:
 *
 * \code
 * MatchingCallbacksRegistry<...>::matchingCallbacks
 * \endcode
 *
 * -----------------------------------------------------------------------------
 * Structural guarantees
 * -----------------------------------------------------------------------------
 *
 * The ordering of the callback table is strictly defined by:
 * - the order of concepts in `detail::AllConcepts`.
 *
 * The index into this table is therefore stable and consistent with:
 * - concept identifiers returned by `GeneralRegistry::conceptId()`,
 * - all downstream registry layers.
 *
 * Any change to the concept list or its ordering constitutes a breaking
 * structural change.
 */
template <class MarsDict_t, class ParDict_t>
struct MatchingCallbacksRegistry {

   /**
     * @brief Fully materialized matching dispatch table.
     *
     * This static data member contains the complete entry-level matching
     * registry for:
     * - all concepts,
     * - a single capability index (`0`),
     * - the dictionary types bound to this registry.
     *
     * The table is generated entirely at compile time by invoking
     * `makeEntryCallbacksRegistry` with:
     * - the full concept universe (`AllConcepts`),
     * - capability index `0`,
     * - the dictionary types bound to this registry.
     *
     * Each entry is either:
     * - a valid matcher function (`Fm`), or
     * - `nullptr` if the concept does not participate in matching.
     *
     * This table is typically consumed by the *semantic resolution* layer
     * to determine which concepts and variants are active for a given
     * input request.
     */
    static constexpr auto matchingCallbacks =
        metkit::mars2grib::backend::compile_time_registry_engine::makeEntryCallbacksRegistry<
            detail::AllConcepts, 0,
            MarsDict_t, ParDict_t
        >();
};

}
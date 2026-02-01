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
 * @file GeneralRegistry.h
 * @brief Canonical compile-time registry instantiation for all mars2grib concepts.
 *
 * This header defines the **single, authoritative instantiation** of the
 * compile-time registry engine over the complete set of semantic concepts
 * known to the mars2grib backend.
 *
 * -----------------------------------------------------------------------------
 * Scope and responsibility
 * -----------------------------------------------------------------------------
 *
 * This file has one precise responsibility:
 *
 *   - to bind the generic `EntryVariantRegistry` template to the concrete,
 *     ordered list of all concept descriptors (`AllConcepts`).
 *
 * It does **not**:
 *   - define any concept descriptors,
 *   - implement any dispatch logic,
 *   - perform any compile-time computation itself,
 *   - introduce any new abstractions.
 *
 * It is purely a **type aliasing and configuration point**.
 *
 * -----------------------------------------------------------------------------
 * Architectural role
 * -----------------------------------------------------------------------------
 *
 * `GeneralRegistry` is the **central structural metadata provider** for the
 * entire mars2grib backend.
 *
 * It exposes, in a single type:
 *
 *   - the complete concept universe,
 *   - the global variant index space,
 *   - all compile-time constants derived from concept structure
 *     (counts, offsets, identifiers),
 *   - constexpr lookup tables for concept and variant metadata.
 *
 * All higher-level systems depend on this registry, including:
 *
 *   - header layout resolution,
 *   - encoding plan construction,
 *   - callback dispatch tables,
 *   - debugging and diagnostic utilities.
 *
 * -----------------------------------------------------------------------------
 * Ordering and stability guarantees
 * -----------------------------------------------------------------------------
 *
 * The structure of `GeneralRegistry` is entirely determined by
 * `detail::AllConcepts`.
 *
 * Consequently:
 *
 *   - the order of concepts in `AllConcepts` defines concept identifiers,
 *   - the order of variants in each concept defines global variant indices,
 *   - any reordering is a **breaking structural change**.
 *
 * Stability of this registry is therefore critical for:
 *   - reproducibility,
 *   - deterministic encoding,
 *   - consistency across translation units.
 *
 * -----------------------------------------------------------------------------
 * Namespace choice
 * -----------------------------------------------------------------------------
 *
 * This registry is placed in the `concepts_` namespace (with trailing
 * underscore) to:
 *
 *   - avoid collision with the C++20 `concepts` keyword,
 *   - clearly distinguish internal structural concepts from language features.
 *
 * -----------------------------------------------------------------------------
 * Design constraints
 * -----------------------------------------------------------------------------
 *
 * - Header-only
 * - Purely compile-time
 * - No runtime state
 * - No dynamic allocation
 *
 * This header must remain lightweight and safe to include transitively.
 */
#pragma once

// System includes
#include <cstdint>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/EntryVariantRegistry.h"
#include "metkit/mars2grib/backend/concepts/AllConcepts.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Canonical compile-time registry over all mars2grib concepts.
 *
 * `GeneralRegistry` is a concrete specialization of
 * `EntryVariantRegistry`, instantiated with the complete, ordered list
 * of all concept descriptors.
 *
 * This alias provides a **single point of access** to:
 *
 *   - concept counts and identifiers,
 *   - variant counts and global indices,
 *   - compile-time offset calculations,
 *   - constexpr metadata tables (names, IDs),
 *   - sentinel-aware index utilities.
 *
 * It is intended to be used as:
 *
 *   - the structural backbone of the encoding pipeline,
 *   - a read-only, constexpr registry,
 *   - a shared dependency across frontend and backend layers.
 *
 * @note
 * There must be exactly one such registry instantiation for the entire
 * mars2grib backend. Introducing additional instantiations with different
 * concept lists would create incompatible index spaces.
 */
using GeneralRegistry = EntryVariantRegistry<detail::AllConcepts>;

}
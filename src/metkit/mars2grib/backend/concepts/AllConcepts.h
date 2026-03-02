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
/// @file AllConcepts.h
/// @brief Canonical compile-time list of all mars2grib semantic concepts.
///
/// This header defines the **complete, ordered universe of semantic concepts**
/// known to the mars2grib backend.
///
/// -----------------------------------------------------------------------------
/// Scope and responsibility
/// -----------------------------------------------------------------------------
///
/// This file has a single, strictly limited responsibility:
///
/// - to aggregate all concept descriptor types into one canonical
/// compile-time `TypeList`.
///
/// It does **not**:
/// - define any concept logic,
/// - implement any dispatch behavior,
/// - perform any registry construction,
/// - introduce any compile-time algorithms.
///
/// All included headers are assumed to provide **concept descriptor types**
/// conforming to the `RegisterEntryDescriptor` contract.
///
/// -----------------------------------------------------------------------------
/// Architectural role
/// -----------------------------------------------------------------------------
///
/// The `AllConcepts` typelist defined in this file is the **root input** to the
/// entire compile-time registry engine.
///
/// It is consumed by:
///
/// - EntryVariantRegistry   (index space definition),
/// - EntryCallbacksRegistry (entry-level dispatch),
/// - VariantCallbacksRegistry (variant-level dispatch),
/// - PhaseCallbacksRegistry (phase-level dispatch).
///
/// The **order** of types in this list is semantically significant and defines:
///
/// - concept identifiers (conceptId),
/// - the layout of flattened variant index spaces,
/// - the ordering of all generated dispatch tables.
///
/// Reordering entries in this list changes global indices and must therefore be
/// treated as a **breaking structural change**.
///
/// -----------------------------------------------------------------------------
/// Design constraints
/// -----------------------------------------------------------------------------
///
/// - Header-only
/// - Purely declarative
/// - No runtime state
/// - No constexpr logic beyond type aggregation
///
/// This file must remain lightweight and stable, as it is included transitively
/// by many translation units.
///
/// -----------------------------------------------------------------------------
/// Extension policy
/// -----------------------------------------------------------------------------
///
/// To introduce a new semantic concept into the system:
///
/// 1. Implement a new concept descriptor conforming to
/// `RegisterEntryDescriptor`.
/// 2. Include its header in this file.
/// 3. Append the descriptor type to the `AllConcepts` typelist.
///
/// No other changes are required to integrate the new concept into the
/// compile-time registry engine.
///

#pragma once

// System includes
#include <cstdint>

// Project includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"

#include "metkit/mars2grib/backend/concepts/analysis/analysisConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/composition/compositionConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/data-type/dataTypeConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/derived/derivedConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/destine/destineConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/ensemble/ensembleConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/generating-process/generatingProcessConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/level/levelConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrangeConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/mars/marsConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/nil/nilConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/origin/originConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/packing/packingConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/param/paramConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/point-in-time/pointInTimeConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/representation/representationConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/satellite/satelliteConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarthConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/tables/tablesConceptDescriptor.h"
#include "metkit/mars2grib/backend/concepts/wave/waveConceptDescriptor.h"


namespace metkit::mars2grib::backend::concepts_::detail {

///
/// @brief Local alias for the canonical compile-time TypeList.
///
/// This alias shortens references to the `TypeList` type provided by the
/// compile-time registry engine and avoids repeating long qualified names
/// within this namespace.
///
/// @tparam Ts
/// Pack of concept descriptor types.
///
template <typename... Ts>
using TypeList = metkit::mars2grib::backend::compile_time_registry_engine::TypeList<Ts...>;

///
/// @brief Complete ordered list of all semantic concept descriptors.
///
/// `AllConcepts` defines the **entire concept universe** known to the
/// mars2grib backend at compile time.
///
/// Each type in this list:
/// - represents one semantic concept,
/// - conforms to `RegisterEntryDescriptor`,
/// - contributes its variants and dispatch interfaces to the registry engine.
///
/// -----------------------------------------------------------------------------
/// Ordering guarantees
/// -----------------------------------------------------------------------------
///
/// The order of types in this list:
///
/// - defines the concept identifier (`conceptId`) assigned to each concept,
/// - determines the layout of global variant indices,
/// - is propagated unchanged into all generated dispatch tables.
///
/// This order must therefore be:
/// - stable,
/// - intentional,
/// - modified only with full awareness of downstream effects.
///
/// -----------------------------------------------------------------------------
/// Visibility
/// -----------------------------------------------------------------------------
///
/// This typelist is placed in the `detail` namespace to:
/// - discourage casual use outside registry construction,
/// - make its role as *structural metadata* explicit.
///
/// Higher-level code should interact with concepts exclusively through
/// registry APIs, not by iterating this list directly.
///
using AllConcepts =
    TypeList<AnalysisConcept, CompositionConcept, DataTypeConcept, DerivedConcept, DestineConcept, EnsembleConcept,
             GeneratingProcessConcept, LevelConcept, LongrangeConcept, MarsConcept, NilConcept, OriginConcept,
             PackingConcept, ParamConcept, PointInTimeConcept, ReferenceTimeConcept, RepresentationConcept,
             SatelliteConcept, ShapeOfTheEarthConcept, StatisticsConcept, TablesConcept, WaveConcept>;

}  // namespace metkit::mars2grib::backend::concepts_::detail
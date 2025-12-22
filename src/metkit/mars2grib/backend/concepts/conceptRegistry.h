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
 * @file conceptRegistry.h
 * @brief Compile-time registration and lookup of mars2grib concept execution tables.
 *
 * This header defines the **concept registry** used by the mars2grib backend
 * to associate:
 *
 * - a **concept name** (e.g. `"analysis"`, `"level"`, `"packing"`)
 * - a **variant name** (e.g. `"Default"`, `"Hybrid"`, `"Spectra"`)
 *
 * with a **fully materialized execution table**:
 *
 *   `[NUM_STAGES × NUM_SECTIONS] → function pointer`
 *
 * The registry is constructed entirely at compile time using template
 * metaprogramming and stores only resolved function pointers at runtime.
 *
 * ## Design principles
 *
 * - **No virtual dispatch**
 * - **No runtime branching on stage/section**
 * - **Full compile-time validation of applicability**
 * - **Deterministic and reproducible concept execution**
 *
 * Each registered concept variant provides a table of callbacks indexed by:
 *
 * - encoding stage (allocate / preset / runtime)
 * - GRIB section
 *
 * Missing or non-applicable entries are valid and resolved at compile time.
 *
 * ## Architecture overview
 *
 * ```
 * (conceptName, variantName)
 *          ↓
 *  +-----------------------+
 *  |  [Stage][Section]     |
 *  |  FnPtr or no-op       |
 *  +-----------------------+
 * ```
 *
 * The registry is populated by explicitly registering all known concepts
 * and their variants via `RegisterVariants`.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

#include <map>
#include <string>


// Core concept includes
#include "metkit/mars2grib/backend/concepts/analysis/analysis.h"
#include "metkit/mars2grib/backend/concepts/composition/composition.h"
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/data-type/dataType.h"
#include "metkit/mars2grib/backend/concepts/derived/derived.h"
#include "metkit/mars2grib/backend/concepts/destine/destine.h"
#include "metkit/mars2grib/backend/concepts/ensemble/ensemble.h"
#include "metkit/mars2grib/backend/concepts/generating-process/generatingProcess.h"
#include "metkit/mars2grib/backend/concepts/level/level.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrange.h"
#include "metkit/mars2grib/backend/concepts/mars/mars.h"
#include "metkit/mars2grib/backend/concepts/nil/nil.h"
#include "metkit/mars2grib/backend/concepts/origin/origin.h"
#include "metkit/mars2grib/backend/concepts/packing/packing.h"
#include "metkit/mars2grib/backend/concepts/param/param.h"
#include "metkit/mars2grib/backend/concepts/point-in-time/pointInTime.h"
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTime.h"
#include "metkit/mars2grib/backend/concepts/representation/representation.h"
#include "metkit/mars2grib/backend/concepts/satellite/satellite.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarth.h"
#include "metkit/mars2grib/backend/concepts/statistics/statistics.h"
#include "metkit/mars2grib/backend/concepts/tables/tables.h"
#include "metkit/mars2grib/backend/concepts/wave/wave.h"


namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Runtime registry mapping concept variants to execution tables.
 *
 * This structure stores a mapping from:
 *
 *   `(conceptName, variantName)`
 *
 * to a **2D execution table** indexed by:
 *
 * - encoding stage (`NUM_STAGES`)
 * - GRIB section (`NUM_SECTIONS`)
 *
 * Each cell contains a function pointer of type `Fn`, which executes
 * the corresponding concept operation.
 *
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @note
 * - The registry itself performs **no logic**.
 * - All applicability decisions are resolved at compile time.
 * - The map is populated once and then treated as read-only.
 */
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
struct ConceptRegistry {

    /**
     * @brief Function pointer type for concept execution.
     *
     * This is the uniform callable signature for all concept operations.
     */
    using FnPtr = Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;

    /**
     * @brief Execution table type.
     *
     * Indexed as:
     *   - first dimension: encoding stage
     *   - second dimension: GRIB section
     */
    using Table = std::array<std::array<FnPtr, NUM_SECTIONS>, NUM_STAGES>;

    /**
     * @brief Registry storage.
     *
     * Key:
     *   `(conceptName, variantName)`
     *
     * Value:
     *   execution table for that concept variant
     */
    std::map<std::pair<std::string_view, std::string_view>, Table> map;

    /**
     * @brief Register a concept variant execution table.
     *
     * @param[in] conceptName Name of the concept (stable identifier)
     * @param[in] variantName Name of the variant (stable identifier)
     * @param[in] table Fully constructed execution table
     *
     * @note
     * - This function is intended to be called only during registry construction.
     * - Duplicate keys are not expected and indicate a programming error.
     */
    void add(const std::string_view& conceptName, const std::string_view& variantName, Table table) {
        map.emplace(std::make_pair(conceptName, variantName), std::move(table));
    }
};


/**
 * @brief Construct a fully populated concept registry.
 *
 * This function registers **all known mars2grib concepts and all their
 * supported variants**, producing a complete runtime registry.
 *
 * Registration is performed by invoking `RegisterVariants` for each concept,
 * which:
 *
 * - iterates over the concept's variant list
 * - generates the `[Stage × Section]` execution table
 * - inserts it into the registry
 *
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @return A fully populated `ConceptRegistry` instance
 *
 * @note
 * - This function is intentionally **not** a singleton.
 * - It allows controlled instantiation for testing or alternative pipelines.
 */
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
ConceptRegistry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> make_concept_registry() {
    using Registry = ConceptRegistry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;

    Registry registry;

    RegisterVariants<NilConceptInfo, NilList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(registry);

    RegisterVariants<OriginConceptInfo, OriginList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<ParamConceptInfo, ParamList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<TablesConceptInfo, TablesList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<DataTypeConceptInfo, DataTypeList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<ReferenceTimeConceptInfo, ReferenceTimeList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    RegisterVariants<MarsConceptInfo, MarsList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(registry);

    RegisterVariants<LongrangeConceptInfo, LongrangeList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<AnalysisConceptInfo, AnalysisList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<DestineConceptInfo, DestineList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<EnsembleConceptInfo, EnsembleList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<LevelConceptInfo, LevelList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<StatisticsConceptInfo, StatisticsList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    RegisterVariants<WaveConceptInfo, WaveList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(registry);

    RegisterVariants<PointInTimeConceptInfo, PointInTimeList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    RegisterVariants<RepresentationConceptInfo, RepresentationList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    RegisterVariants<GeneratingProcessConceptInfo, GeneratingProcessList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    RegisterVariants<ShapeOfTheEarthConceptInfo, ShapeOfTheEarthList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    RegisterVariants<PackingConceptInfo, PackingList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<SatelliteConceptInfo, SatelliteList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>::run(
        registry);

    RegisterVariants<CompositionConceptInfo, CompositionList, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t,
                     OutDict_t>::run(registry);

    return registry;
}


/**
 * @brief Access the global concept registry singleton.
 *
 * This function returns a lazily initialized, process-wide
 * concept registry instance.
 *
 * The registry is constructed exactly once, on first use,
 * and reused for all subsequent accesses.
 *
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @return Reference to the singleton `ConceptRegistry`
 *
 * @note
 * - Thread-safe since C++11 (static local initialization).
 * - Registry contents are immutable after construction.
 */
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
ConceptRegistry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>& concept_registry_instance() {
    static auto instance = make_concept_registry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>();

    return instance;
}

}  // namespace metkit::mars2grib::backend::concepts_

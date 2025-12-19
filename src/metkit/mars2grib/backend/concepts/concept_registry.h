#pragma once

#include <array>
#include <map>
#include <string>
#include <utility>

#include "metkit/mars2grib/backend/concepts/concept_core.h"


#include "metkit/mars2grib/backend/concepts/analysis/analysis.h"
// Missing Composition
#include "metkit/mars2grib/backend/concepts/data-type/data_type.h"
// Missing derived
#include "metkit/mars2grib/backend/concepts/composition/composition.h"
#include "metkit/mars2grib/backend/concepts/destine/destine.h"
#include "metkit/mars2grib/backend/concepts/ensemble/ensemble.h"
#include "metkit/mars2grib/backend/concepts/generating-process/generating_process.h"
#include "metkit/mars2grib/backend/concepts/level/level.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrange.h"
#include "metkit/mars2grib/backend/concepts/mars/mars.h"
#include "metkit/mars2grib/backend/concepts/nil/nil.h"
#include "metkit/mars2grib/backend/concepts/origin/origin.h"
#include "metkit/mars2grib/backend/concepts/packing/packing.h"
#include "metkit/mars2grib/backend/concepts/param/param.h"
#include "metkit/mars2grib/backend/concepts/point-in-time/point_in_time.h"
#include "metkit/mars2grib/backend/concepts/reference-time/reference_time.h"
#include "metkit/mars2grib/backend/concepts/representation/representation.h"
#include "metkit/mars2grib/backend/concepts/satellite/satellite.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shape_of_the_earth.h"
#include "metkit/mars2grib/backend/concepts/statistics/statistics.h"
#include "metkit/mars2grib/backend/concepts/tables/tables.h"
#include "metkit/mars2grib/backend/concepts/wave/wave.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// Registry: (concept, variantName) -> table [NUM_STAGES x NUM_SECTIONS]
// ======================================================
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
struct ConceptRegistry {
    using FnPtr = Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
    using Table = std::array<std::array<FnPtr, NUM_SECTIONS>, NUM_STAGES>;

    std::map<std::pair<std::string_view, std::string_view>, Table> map;

    void add(const std::string_view& conceptName, const std::string_view& variantName, Table table) {
        map.emplace(std::make_pair(conceptName, variantName), std::move(table));
    }
};


// ======================================================
// make_concept_registry()  (NON-SINGLETON)
// ======================================================
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

// ======================================================
// concept_registry_instance()  (SINGLETON)
// ======================================================
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
ConceptRegistry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>& concept_registry_instance() {
    static auto instance = make_concept_registry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>();

    return instance;
}

}  // namespace metkit::mars2grib::backend::cnpts

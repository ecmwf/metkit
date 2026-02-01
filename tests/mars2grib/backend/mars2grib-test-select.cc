#include <iostream>
#include <string>

#include "eckit/testing/Test.h"

#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/frontend/resolution/recipes/Select.h"

// include all concepts
#include "metkit/mars2grib/backend/concepts/test.h"

using namespace metkit::mars2grib::frontend::resolution::recipes;
using namespace metkit::mars2grib::backend::concepts_;

namespace {

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
template <typename SelectT>
void dump_ids(const SelectT&) {
    std::string sep = "[ ";
    for (auto v : SelectT::ids) {
        std::cout << sep << v;
        sep = ", ";
    }
    std::cout << " ]";
}

template <typename Concept>
void check_select_any(const char* name) {

    using S = Select<Concept>;

    CASE(std::string("Select<") + name + ">: all variants") {

        EXPECT(S::is_any);
        EXPECT(S::count == Concept::variant_count);

        std::cout << "Select<" << name << "> ids = ";
        dump_ids(S{});
        std::cout << std::endl;
    }
}

template <typename Concept, auto... Vs>
void check_select_subset(const char* name) {

    using S = Select<Concept, Vs...>;

    CASE(std::string("Select<") + name + ">: subset") {

        EXPECT(!S::is_any);
        EXPECT(S::count == sizeof...(Vs));

        std::cout << "Select<" << name << ", subset> ids = ";
        dump_ids(S{});
        std::cout << std::endl;
    }
}

}  // namespace

// -----------------------------------------------------------------------------
// Tests per concept
// -----------------------------------------------------------------------------

// ---- AnalysisConcept --------------------------------------------------------
check_select_any<AnalysisConcept>("AnalysisConcept");
// TODO: fill enumerators
// check_select_subset<AnalysisConcept, AnalysisType::XXX, AnalysisType::YYY>("AnalysisConcept");

// ---- CompositionConcept -----------------------------------------------------
check_select_any<CompositionConcept>("CompositionConcept");
// TODO
// check_select_subset<CompositionConcept, CompositionType::Chem>("CompositionConcept");

// ---- DataTypeConcept --------------------------------------------------------
check_select_any<DataTypeConcept>("DataTypeConcept");
// TODO
// check_select_subset<DataTypeConcept, DataType::XXX>("DataTypeConcept");

// ---- DerivedConcept ---------------------------------------------------------
check_select_any<DerivedConcept>("DerivedConcept");
// TODO
// check_select_subset<DerivedConcept, DerivedType::XXX>("DerivedConcept");

// ---- DestineConcept ---------------------------------------------------------
check_select_any<DestineConcept>("DestineConcept");
// TODO

// ---- EnsembleConcept --------------------------------------------------------
check_select_any<EnsembleConcept>("EnsembleConcept");
// TODO
// check_select_subset<EnsembleConcept, EnsembleType::Individual>("EnsembleConcept");

// ---- GeneratingProcessConcept ----------------------------------------------
check_select_any<GeneratingProcessConcept>("GeneratingProcessConcept");
// TODO

// ---- LevelConcept -----------------------------------------------------------
check_select_any<LevelConcept>("LevelConcept");
// TODO
// check_select_subset<LevelConcept,
//     LevelType::Surface,
//     LevelType::HeightAboveGround>("LevelConcept");

// ---- LongrangeConcept -------------------------------------------------------
check_select_any<LongrangeConcept>("LongrangeConcept");
// TODO

// ---- MarsConcept ------------------------------------------------------------
check_select_any<MarsConcept>("MarsConcept");
// TODO

// ---- NilConcept -------------------------------------------------------------
check_select_any<NilConcept>("NilConcept");
// TODO

// ---- OriginConcept ----------------------------------------------------------
check_select_any<OriginConcept>("OriginConcept");
// TODO

// ---- PackingConcept ---------------------------------------------------------
check_select_any<PackingConcept>("PackingConcept");
// TODO

// ---- ParamConcept -----------------------------------------------------------
check_select_any<ParamConcept>("ParamConcept");
// TODO
// check_select_subset<ParamConcept, 130, 131>("ParamConcept");

// ---- PointInTimeConcept -----------------------------------------------------
check_select_any<PointInTimeConcept>("PointInTimeConcept");
// TODO

// ---- ReferenceTimeConcept ---------------------------------------------------
check_select_any<ReferenceTimeConcept>("ReferenceTimeConcept");
// TODO

// ---- RepresentationConcept -------------------------------------------------
check_select_any<RepresentationConcept>("RepresentationConcept");
// TODO

// ---- SatelliteConcept -------------------------------------------------------
check_select_any<SatelliteConcept>("SatelliteConcept");
// TODO

// ---- ShapeOfTheEarthConcept -------------------------------------------------
check_select_any<ShapeOfTheEarthConcept>("ShapeOfTheEarthConcept");
// TODO

// ---- StatisticsConcept ------------------------------------------------------
check_select_any<StatisticsConcept>("StatisticsConcept");
// TODO

// ---- TablesConcept ----------------------------------------------------------
check_select_any<TablesConcept>("TablesConcept");
// TODO

// ---- WaveConcept ------------------------------------------------------------
check_select_any<WaveConcept>("WaveConcept");
// TODO
// check_select_subset<WaveConcept, WaveType::Spectra>("WaveConcept");

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

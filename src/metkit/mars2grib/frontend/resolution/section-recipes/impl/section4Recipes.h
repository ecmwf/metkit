//------------------------------------------------------------------------------
// Section 4 Recipes (runtime, static inline)
//------------------------------------------------------------------------------

// All elements needed to used the "dsl" objects to define recipes
#include "metkit/mars2grib/backend/sections/resolver/dsl.h"

namespace metkit::mars2grib::frontend::resolution::recipes::impl {

using namespace metkit::mars2grib::backend::concepts_;
using namespace metkit::mars2grib::backend::sections::resolver::dsl;

// clang-format off

//------------------------------------------------------------------------------
// Section 4 – Individual Recipes
//------------------------------------------------------------------------------

inline const Recipe S4_R0 =
    make_recipe<0,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R1 =
    make_recipe<1,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R2 =
    make_recipe<2,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>,
        Select<DerivedConcept>
    >();

inline const Recipe S4_R8 =
    make_recipe<8,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R11 =
    make_recipe<11,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R12 =
    make_recipe<12,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>,
        Select<DerivedConcept>
    >();

inline const Recipe S4_R32 =
    make_recipe<32,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<SatelliteConcept>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R33 =
    make_recipe<33,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<SatelliteConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R40 =
    make_recipe<40,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Chem>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R41 =
    make_recipe<41,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Chem>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R42 =
    make_recipe<42,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Chem>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R43 =
    make_recipe<43,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Chem>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R50 =
    make_recipe<50,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Aerosol>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R45 =
    make_recipe<45,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Aerosol>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R46 =
    make_recipe<46,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Aerosol>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R85 =
    make_recipe<85,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::Aerosol>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R48 =
    make_recipe<48,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::AerosolOptical>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R49 =
    make_recipe<49,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::AerosolOptical>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R60 =
    make_recipe<60,
        Select<GeneratingProcessConcept>,
        Select<ReferenceTimeConcept, ReferenceTimeType::Reforecast>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R61 =
    make_recipe<61,
        Select<GeneratingProcessConcept>,
        Select<ReferenceTimeConcept, ReferenceTimeType::Reforecast>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R76 =
    make_recipe<76,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::ChemicalSource>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R77 =
    make_recipe<77,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::ChemicalSource>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R78 =
    make_recipe<78,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::ChemicalSource>,
        Select<ParamConcept>
    >();

inline const Recipe S4_R79 =
    make_recipe<79,
        Select<GeneratingProcessConcept>,
        Select<StatisticsConcept>,
        Select<LevelConcept>,
        Select<CompositionConcept, CompositionType::ChemicalSource>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R99 =
    make_recipe<99,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<ParamConcept>,
        Select<WaveConcept, WaveType::Spectra>
    >();

inline const Recipe S4_R100 =
    make_recipe<100,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<ParamConcept>,
        Select<WaveConcept, WaveType::Spectra>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R103 =
    make_recipe<103,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<ParamConcept>,
        Select<WaveConcept, WaveType::Period>
    >();

inline const Recipe S4_R104 =
    make_recipe<104,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<ParamConcept>,
        Select<WaveConcept, WaveType::Period>,
        Select<EnsembleConcept, EnsembleType::Individual>
    >();

inline const Recipe S4_R142 =
    make_recipe<142,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::PerturbedParameters>
    >();

inline const Recipe S4_R143 =
    make_recipe<143,
        Select<GeneratingProcessConcept>,
        Select<PointInTimeConcept>,
        Select<ParamConcept>,
        Select<EnsembleConcept, EnsembleType::RandomPatterns>
    >();

//------------------------------------------------------------------------------
// Section 4 – Aggregated Recipes
//------------------------------------------------------------------------------

inline const Recipes Section4Recipes{ 4,
    std::vector<const Recipe*>{
        &S4_R0,  &S4_R1,  &S4_R2,
        &S4_R8,  &S4_R11, &S4_R12,
        &S4_R32, &S4_R33,
        &S4_R40, &S4_R41, &S4_R42, &S4_R43,
        &S4_R50, &S4_R45, &S4_R46, &S4_R85,
        &S4_R48, &S4_R49,
        &S4_R60, &S4_R61,
        &S4_R76, &S4_R77, &S4_R78, &S4_R79,
        &S4_R99, &S4_R100,
        &S4_R103, &S4_R104,
        &S4_R142, &S4_R143
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

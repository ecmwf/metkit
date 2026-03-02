//------------------------------------------------------------------------------
// Section 2 Recipes (runtime, static inline)
//------------------------------------------------------------------------------

#pragma once

// All elements needed to used the "dsl" objects to define recipes
#include "metkit/mars2grib/backend/sections/resolver/dsl.h"

namespace metkit::mars2grib::frontend::resolution::recipes::impl {

using namespace metkit::mars2grib::backend::concepts_;
using namespace metkit::mars2grib::backend::sections::resolver::dsl;

// clang-format off

//------------------------------------------------------------------------------
// Section 2 – Individual Recipes
//------------------------------------------------------------------------------

// Standard local definition
inline const Recipe S2_R1 =
    make_recipe<1,
        Select<MarsConcept>
    >();

// Long-range products
inline const Recipe S2_R15 =
    make_recipe<15,
        Select<MarsConcept>,
        Select<LongrangeConcept>
    >();

// Satellite-related products
inline const Recipe S2_R24 =
    make_recipe<24,
        Select<MarsConcept>,
        Select<SatelliteConcept>
    >();

// Analysis-related products
inline const Recipe S2_R36 =
    make_recipe<36,
        Select<MarsConcept>,
        Select<AnalysisConcept>
    >();

//------------------------------------------------------------------------------
// Virtual (encoder-specific) templates
//------------------------------------------------------------------------------

// DestinE Climate DT products
inline const Recipe S2_R1001 =
    make_recipe<1001,
        Select<MarsConcept>,
        Select<DestineConcept, DestineType::ClimateDT>
    >();

// DestinE Extremes DT products
inline const Recipe S2_R1002 =
    make_recipe<1002,
        Select<MarsConcept>,
        Select<DestineConcept, DestineType::ExtremesDT>
    >();

// DestinE On-demand Extremes DT products
//inline const Recipe S2_R1004 =
//    make_recipe<1004,
//        Select<MarsConcept>,
//        Select<DestineConcept, DestineType::OnDemandExtremesDT>
//    >();

//------------------------------------------------------------------------------
// Section 2 – Aggregated Recipes
//------------------------------------------------------------------------------

inline const Recipes Section2Recipes{ 2,
    std::vector<const Recipe*>{
        &S2_R1,
        &S2_R15,
        &S2_R24,
        &S2_R36,
        &S2_R1001,
        &S2_R1002
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

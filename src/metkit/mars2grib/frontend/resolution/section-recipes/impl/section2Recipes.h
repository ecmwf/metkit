//------------------------------------------------------------------------------
// Section 2 Recipes (runtime, static inline)
//------------------------------------------------------------------------------

#pragma once

// All elements needed to used the "dsl" objects to define recipes
#include "metkit/mars2grib/backend/sections/resolver/dsl.h"
#include "metkit/mars2grib/utils/generalUtils.h"

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
        Select<LongrangeConcept, LongrangeType::SeasonalForecast>
    >();

// Long-range products
inline const Recipe S2_R16 =
    make_recipe<16,
        Select<MarsConcept>,
        Select<LongrangeConcept, LongrangeType::SeasonalForecastMonthlyMean>
    >();

// 4i related products
inline const Recipe S2_R20 =
    make_recipe<20,
        Select<MarsConcept>,
        Select<IterationConcept>
    >();

// Satellite-related products
inline const Recipe S2_R24 =
    make_recipe<24,
        Select<MarsConcept>,
        Select<SatelliteConcept>
    >();

// 4DVar model errors
inline const Recipe S2_R25 =
    make_recipe<25,
        Select<MarsConcept>,
        Select<ModelErrorConcept, ModelErrorType::ComponentIndex>
    >();

// Analysis-related products
inline const Recipe S2_R36 =
    make_recipe<36,
        Select<MarsConcept>,
        Select<AnalysisConcept>
    >();

// Brightness temperature satellite products
inline const Recipe S2_R37A =
    make_recipe<37,
        Select<MarsConcept>,
        Select<AnalysisConcept>,
        Select<BrightnessTemperatureConcept, BrightnessTemperatureType::EnsembleMean>
    >();

inline const Recipe S2_R37B =
    make_recipe<37,
        Select<MarsConcept>,
        Select<AnalysisConcept>,
        Select<SatelliteConcept>,
        Select<BrightnessTemperatureConcept, BrightnessTemperatureType::Default>
    >();

// 4i Analysis-related products
inline const Recipe S2_R38 =
    make_recipe<38,
        Select<MarsConcept>,
        Select<IterationConcept>,
        Select<AnalysisConcept>
    >();

// 4DVar model errors for long window 4Dvar system
inline const Recipe S2_R39 =
    make_recipe<39,
        Select<MarsConcept>,
        Select<AnalysisConcept>,
        Select<ModelErrorConcept, ModelErrorType::ComponentIndex>
    >();

// Model error fourier coefficients
// Note: Template 45 has the analysis concept, but it's unused (and unmatched) for stream oper
inline const Recipe S2_R45 =
    make_recipe<45,
        Select<MarsConcept>,
        Select<ModelErrorConcept, ModelErrorType::FourierCoefficients>
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
        &S2_R16,
        &S2_R20,
        &S2_R24,
        &S2_R25,
        &S2_R36,
        &S2_R37A,
        &S2_R37B,
        &S2_R38,
        &S2_R39,
        &S2_R45,
        &S2_R1001,
        &S2_R1002
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

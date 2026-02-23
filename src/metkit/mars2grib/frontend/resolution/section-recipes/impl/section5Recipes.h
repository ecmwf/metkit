//------------------------------------------------------------------------------
// Section 5 Recipes (runtime, static inline)
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
// Section 5 – Individual Recipes
//------------------------------------------------------------------------------

// Simple packing
inline const Recipe S5_R0 =
    make_recipe<0,
        Select<PackingConcept, PackingType::Simple>
    >();

// CCSDS compression
inline const Recipe S5_R42 =
    make_recipe<42,
        Select<PackingConcept, PackingType::Ccsds>
    >();

// Spectral complex packing
inline const Recipe S5_R51 =
    make_recipe<51,
        Select<PackingConcept, PackingType::SpectralComplex>
    >();

//------------------------------------------------------------------------------
// Section 5 – Aggregated Recipes
//------------------------------------------------------------------------------

inline const Recipes Section5Recipes{ 5,
    std::vector<const Recipe*>{
        &S5_R0,
        &S5_R42,
        &S5_R51
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

//------------------------------------------------------------------------------
// Section 0 Recipes (runtime, static inline)
//------------------------------------------------------------------------------

#pragma once

// All elements needed to used the "dsl" objects to define recipes
#include "metkit/mars2grib/backend/sections/resolver/dsl.h"

namespace metkit::mars2grib::frontend::resolution::recipes::impl {

using namespace metkit::mars2grib::backend::concepts_;
using namespace metkit::mars2grib::backend::sections::resolver::dsl;

// clang-format off

//------------------------------------------------------------------------------
// Section 0 – Individual Recipes
//------------------------------------------------------------------------------

// Indicator Section (sentinel-only)
inline const Recipe S0_R0 =
    make_recipe<0,
        Select<NilConcept>
    >();

//------------------------------------------------------------------------------
// Section 0 – Aggregated Recipes
//------------------------------------------------------------------------------

inline const Recipes Section0Recipes{ 0,
    std::vector<const Recipe*>{
        &S0_R0
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

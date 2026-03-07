//------------------------------------------------------------------------------
// Section 1 Recipes (runtime, static inline)
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
// Section 1 – Individual Recipes
//------------------------------------------------------------------------------

// Identification Section
inline const Recipe S1_R0 =
    make_recipe<0,
        Select<OriginConcept>,
        Select<TablesConcept>,
        Select<ReferenceTimeConcept>,
        Select<DataTypeConcept>
    >();

//------------------------------------------------------------------------------
// Section 1 – Aggregated Recipes
//------------------------------------------------------------------------------

inline const Recipes Section1Recipes{ 1,
    std::vector<const Recipe*>{
        &S1_R0
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

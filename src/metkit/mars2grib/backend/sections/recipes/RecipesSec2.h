#pragma once

#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

inline const std::vector<SectionRecipe> Sec2Recipes = {{1, {C("mars")}},
                                                       {15, {C("mars"), C("longrange")}},
                                                       {24, {C("mars"), C("satellite")}},
                                                       {36, {C("mars"), C("analysis")}},
                                                       {1001, {C("mars"), C("destine", "climateDT")}},
                                                       {1002, {C("mars"), C("destine", "extremesDT")}},
                                                       {1004, {C("mars"), C("destine", "onDemandExtremesDT")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

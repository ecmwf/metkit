#pragma once

#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

inline const std::vector<SectionRecipe> Sec1Recipes = {
    {0, {C("origin"), C("tables"), C("referenceTime"), C("dataType")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

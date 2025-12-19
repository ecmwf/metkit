#pragma once

#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

inline const std::vector<SectionRecipe> Sec5Recipes = {
    {0, {C("packing", "simple")}}, {42, {C("packing", "ccsds")}}, {51, {C("packing", "spectral_complex")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

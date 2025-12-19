#pragma once


#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

inline const std::vector<SectionRecipe> Sec3Recipes = {

    {0, {C("shapeOfTheEarth"), C("representation", "latlon")}},
    {40, {C("shapeOfTheEarth"), C("representation")}},
    {50, {C("representation", "sphericalHarmonics")}},
    {101, {C("shapeOfTheEarth"), C("representation", "generalUnstructured")}},
    {150, {C("shapeOfTheEarth"), C("representation", "healpix")}}

};

}  // namespace metkit::mars2grib::backend::sections::recipes

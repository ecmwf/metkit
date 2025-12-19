#pragma once

// Datatype to define a recipe
#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

// Definition of recipes for each section
#include "metkit/mars2grib/backend/sections/recipes/RecipesSec0.h"
#include "metkit/mars2grib/backend/sections/recipes/RecipesSec1.h"
#include "metkit/mars2grib/backend/sections/recipes/RecipesSec2.h"
#include "metkit/mars2grib/backend/sections/recipes/RecipesSec3.h"
#include "metkit/mars2grib/backend/sections/recipes/RecipesSec4.h"
#include "metkit/mars2grib/backend/sections/recipes/RecipesSec5.h"

namespace metkit::mars2grib::backend::sections::recipes {

inline const std::vector<SectionRecipe>* recipesForSection(uint16_t sectionId) {
    switch (sectionId) {
        case 0:
            return &Sec0Recipes;
        case 1:
            return &Sec1Recipes;
        case 2:
            return &Sec2Recipes;
        case 3:
            return &Sec3Recipes;
        case 4:
            return &Sec4Recipes;
        case 5:
            return &Sec5Recipes;
        default:
            return nullptr;
    }
}


// TODO MIVAL:: If we can enforce that evey maintainer will always keep the recipes sorted by templateNumber,
// we could implement a binary search here instead of a linear search.
inline const SectionRecipe* findRecipe(uint16_t sectionId, uint16_t templateNumber) {
    const auto* recipes = recipesForSection(sectionId);
    if (!recipes)
        return nullptr;

    for (const auto& r : *recipes) {
        if (r.templateNumber == templateNumber)
            return &r;
    }
    return nullptr;
}

}  // namespace metkit::mars2grib::backend::sections::recipes

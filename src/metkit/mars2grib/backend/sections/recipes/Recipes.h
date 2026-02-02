/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file RecipesRegistry.h
 * @brief Registry and lookup utilities for GRIB section recipes.
 *
 * This header defines the **recipe registry layer** of the mars2grib backend.
 *
 * A *recipe* describes how a given GRIB section and template should be
 * populated, typically by listing the sequence of concept-driven operations
 * required to fill the section consistently.
 *
 * This file provides:
 * - a centralized mapping from GRIB section numbers to their recipe collections
 * - a lookup utility to resolve a recipe by *(section, template number)*
 *
 * Recipes are defined per section in dedicated headers (`RecipesSecX.h`)
 * and aggregated here for uniform access by the encoder.
 *
 * This file contains **no encoding logic** and **no state mutation**.
 * It is purely a dispatch and lookup facility.
 *
 * @ingroup mars2grib_backend_sections
 */
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

/**
 * @brief Retrieve the recipe list associated with a GRIB section.
 *
 * This function returns the collection of recipes defined for the
 * specified GRIB section number.
 *
 * Each recipe collection is defined in a dedicated section-specific
 * header and exposed here through a uniform interface.
 *
 * @param sectionId GRIB section number
 *
 * @return Pointer to the vector of recipes for the given section,
 *         or `nullptr` if the section is not supported.
 */
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


/**
 * @brief Find a recipe for a given section and template number.
 *
 * This function performs a lookup for a specific recipe matching
 * the provided *(section, template number)* pair.
 *
 * The lookup is currently implemented as a linear search over the
 * section recipe list.
 *
 * @param sectionId      GRIB section number
 * @param templateNumber GRIB template number
 *
 * @return Pointer to the matching recipe, or `nullptr` if no recipe
 *         is found or the section is unsupported.
 *
 * @note
 * If recipe lists are guaranteed to be sorted by `templateNumber`,
 * this function could be optimized to use a binary search.
 */
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

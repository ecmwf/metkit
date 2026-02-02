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
 * @file RecipesSec0.h
 * @brief Recipe definitions for GRIB Section 0.
 *
 * This header defines the **recipe set** for GRIB **Section 0**
 * (Indicator Section) in the mars2grib backend.
 *
 * Section 0 contains only structural metadata related to the GRIB
 * message itself (edition number, total length, etc.) and does not
 * carry any scientific or domain-specific information.
 *
 * As a consequence, its recipe is minimal and consists of a single
 * placeholder concept:
 * - `nil`, indicating that no concept-driven encoding is required
 *
 * The presence of a recipe for Section 0 allows it to participate
 * uniformly in the section/recipe dispatch infrastructure, even
 * though no actual encoding logic is performed.
 *
 * This file is purely declarative and contains **no runtime logic**.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once


#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

/**
 * @brief Recipe list for GRIB Section 0.
 *
 * The list contains a single recipe associated with template number `0`.
 * The recipe declares a single `nil` concept, serving as a sentinel
 * to indicate that no encoding steps are required for this section.
 */
inline const std::vector<SectionRecipe> Sec0Recipes = {{0, {C("nil")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

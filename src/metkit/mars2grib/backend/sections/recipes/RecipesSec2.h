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
 * @file RecipesSec2.h
 * @brief Recipe definitions for GRIB Section 2 (Local Use Section).
 *
 * This header defines the **recipe set** for GRIB **Section 2**
 * (Local Use Section) in the mars2grib backend.
 *
 * Section 2 is used to encode centre-specific or application-specific
 * metadata that is not part of the official GRIB specification.
 *
 * In the mars2grib architecture, Section 2 recipes are primarily driven
 * by the `mars` concept and may be extended with additional concepts
 * depending on the selected local definition (template number).
 *
 * This file is purely declarative and contains **no encoding logic**.
 * It specifies which concepts participate in populating Section 2
 * for each supported local definition template.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once

#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

/**
 * @brief Recipe list for GRIB Section 2 (Local Use Section).
 *
 * Each recipe is associated with a local definition (template) number
 * and declares the ordered set of concepts used to populate Section 2.
 *
 * Supported templates:
 * - `1`    : Standard local definition using only the `mars` concept
 * - `15`   : Long-range products (`mars`, `longrange`)
 * - `24`   : Satellite-related products (`mars`, `satellite`)
 * - `36`   : Analysis-related products (`mars`, `analysis`)
 *
 * Virtual (encoder-specific) templates:
 * - `1001` : DestinE Climate DT products
 *            (`mars`, `destine` with type `climateDT`)
 * - `1002` : DestinE Extremes DT products
 *            (`mars`, `destine` with type `extremesDT`)
 * - `1004` : DestinE On-demand Extremes DT products
 *            (`mars`, `destine` with type `onDemandExtremesDT`)
 *
 * These virtual template numbers are not part of the official GRIB
 * specification and are mapped internally to valid local definitions
 * by the section initializer layer.
 */
inline const std::vector<SectionRecipe> Sec2Recipes = {{1, {C("mars")}},
                                                       {15, {C("mars"), C("longrange")}},
                                                       {24, {C("mars"), C("satellite")}},
                                                       {36, {C("mars"), C("analysis")}},
                                                       {1001, {C("mars"), C("destine", "climateDT")}},
                                                       {1002, {C("mars"), C("destine", "extremesDT")}},
                                                       {1004, {C("mars"), C("destine", "onDemandExtremesDT")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

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
 * @file RecipesSec3.h
 * @brief Recipe definitions for GRIB Section 3 (Grid Definition Section).
 *
 * This header defines the **recipe set** for GRIB **Section 3**, which describes
 * the geometry and representation of the data grid.
 *
 * In the mars2grib backend, Section 3 is populated by combining:
 * - the `shapeOfTheEarth` concept, defining the reference ellipsoid/sphere
 * - the `representation` concept, defining the grid or spectral representation
 *
 * The specific combination and variant of concepts depends on the
 * *Grid Definition Template Number* (GDT).
 *
 * This file is purely declarative and contains **no encoding logic**.
 * It specifies which concepts participate in populating Section 3
 * and the semantic variant to be applied for each supported template.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once


#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {


/**
 * @brief Recipe list for GRIB Section 3 (Grid Definition Section).
 *
 * Each recipe is associated with a grid definition template number
 * and declares the ordered list of concepts used to populate Section 3.
 *
 * Supported templates:
 * - `0`   : Latitude/Longitude grid
 *           (`shapeOfTheEarth`, `representation` = `latlon`)
 * - `40`  : Reduced or Gaussian grid
 *           (`shapeOfTheEarth`, `representation` = `default`)
 * - `50`  : Spectral representation
 *           (`representation` = `sphericalHarmonics`)
 * - `101` : General unstructured grid
 *           (`shapeOfTheEarth`, `representation` = `generalUnstructured`)
 * - `150` : HEALPix grid
 *           (`shapeOfTheEarth`, `representation` = `healpix`)
 *
 * The order of concepts reflects the intended execution order during
 * section initialization and encoding.
 */
inline const std::vector<SectionRecipe> Sec3Recipes = {

    {0, {C("shapeOfTheEarth"), C("representation", "latlon")}},
    {40, {C("shapeOfTheEarth"), C("representation")}},
    {50, {C("representation", "sphericalHarmonics")}},
    {101, {C("shapeOfTheEarth"), C("representation", "generalUnstructured")}},
    {150, {C("shapeOfTheEarth"), C("representation", "healpix")}}

};

}  // namespace metkit::mars2grib::backend::sections::recipes

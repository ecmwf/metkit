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
 * @file RecipesSec5.h
 * @brief Recipe definitions for GRIB Section 5 (Data Representation Section).
 *
 * This header defines the **recipe set** for GRIB **Section 5**, which controls
 * how data values are encoded and packed in the GRIB message.
 *
 * Section 5 specifies the *Data Representation Template* (DRT), determining
 * the packing algorithm and numerical representation used for the field values
 * (e.g. simple packing, CCSDS compression, spectral packing).
 *
 * In the mars2grib backend, Section 5 is populated exclusively through the
 * `packing` concept, with different concept variants corresponding to
 * different data representation templates.
 *
 * This file is purely declarative and contains **no encoding logic**.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once

#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

/**
 * @brief Recipe list for GRIB Section 5 (Data Representation Section).
 *
 * Each recipe associates a Data Representation Template (DRT) number
 * with the appropriate `packing` concept variant.
 *
 * Supported templates:
 * - `0`  : Simple packing
 *          (`packing` = `simple`)
 * - `42` : CCSDS compression
 *          (`packing` = `ccsds`)
 * - `51` : Spectral complex packing
 *          (`packing` = `spectral_complex`)
 *
 * The selected packing variant determines how numerical values are
 * compressed and represented in the GRIB message.
 */
inline const std::vector<SectionRecipe> Sec5Recipes = {
    {0, {C("packing", "simple")}}, {42, {C("packing", "ccsds")}}, {51, {C("packing", "spectral_complex")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

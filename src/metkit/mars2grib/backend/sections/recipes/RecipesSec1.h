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
 * @file RecipesSec1.h
 * @brief Recipe definitions for GRIB Section 1.
 *
 * This header defines the **recipe set** for GRIB **Section 1**
 * (Identification Section) in the mars2grib backend.
 *
 * Section 1 contains metadata identifying the origin, production context,
 * reference time, and general data classification of the GRIB message.
 *
 * In the mars2grib architecture, this section is populated through a
 * fixed sequence of concepts that extract and encode the relevant
 * information from the frontend dictionaries.
 *
 * This file is purely declarative and contains **no encoding logic**.
 * It only specifies which concepts participate in populating Section 1
 * and in which order they are applied.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once

#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

/**
 * @brief Recipe list for GRIB Section 1.
 *
 * The list contains a single recipe associated with template number `0`.
 *
 * The recipe applies the following concepts, in order:
 * - `origin`        : identifies the producing centre and sub-centre
 * - `tables`        : selects GRIB master/local tables
 * - `referenceTime` : encodes the reference (analysis/forecast) time
 * - `dataType`      : specifies the general type of data
 *
 * The order of concepts reflects the intended execution order during
 * section initialization and encoding.
 */
inline const std::vector<SectionRecipe> Sec1Recipes = {
    {0, {C("origin"), C("tables"), C("referenceTime"), C("dataType")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

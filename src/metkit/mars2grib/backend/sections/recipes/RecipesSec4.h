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
 * @file RecipesSec4.h
 * @brief Recipe definitions for GRIB Section 4 (Product Definition Section).
 *
 * This header defines the **recipe set** for GRIB **Section 4**, which encodes
 * the scientific meaning of the data: forecast/analysis type, temporal
 * characteristics, vertical level, parameter, ensemble information, and
 * optional composition or wave semantics.
 *
 * Section 4 is the most semantically rich section in a GRIB message.
 * In the mars2grib backend, its population is driven entirely by
 * *recipes*, which declaratively specify the ordered set of concepts
 * required for each **Product Definition Template (PDT)**.
 *
 * Each recipe listed here corresponds to a GRIB2 PDT number and
 * defines:
 * - which concepts participate
 * - which variant of a concept is used (when applicable)
 * - the execution order of concepts during encoding
 *
 * This file is purely declarative and contains **no encoding logic**.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once


#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {


/**
 * @brief Recipe list for GRIB Section 4 (Product Definition Section).
 *
 * Each entry associates a Product Definition Template (PDT) number
 * with the ordered list of concepts required to populate Section 4.
 *
 * High-level concept groups used in this section include:
 * - `generatingProcess` : forecast/analysis process metadata
 * - `pointInTime` / `statistics` : temporal interpretation of the data
 * - `referenceTime`     : reference time variants (e.g. reforecast)
 * - `level`             : vertical level definition
 * - `param`             : meteorological parameter
 * - `ensemble`          : ensemble semantics and member selection
 * - `derived`           : derived or post-processed products
 * - `composition`       : chemical, aerosol, or source composition
 * - `satellite`         : satellite-specific products
 * - `wave`              : wave spectra or wave period products
 *
 * The order of concepts in each recipe is significant and reflects
 * the intended execution order during encoding.
 */
inline const std::vector<SectionRecipe> Sec4Recipes = {

    {0, {C("generatingProcess"), C("pointInTime"), C("level"), C("param")}},

    {1, {C("generatingProcess"), C("pointInTime"), C("level"), C("param"), C("ensemble", "individual")}},

    {2, {C("generatingProcess"), C("pointInTime"), C("level"), C("param"), C("derived")}},

    {8, {C("generatingProcess"), C("statistics"), C("level"), C("param")}},

    {11, {C("generatingProcess"), C("statistics"), C("level"), C("param"), C("ensemble", "individual")}},

    {12, {C("generatingProcess"), C("statistics"), C("level"), C("param"), C("derived")}},

    {32, {C("generatingProcess"), C("pointInTime"), C("satellite"), C("param")}},

    {33, {C("generatingProcess"), C("pointInTime"), C("satellite"), C("param"), C("ensemble", "individual")}},

    {40, {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "chemical"), C("param")}},

    {41,
     {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "chemical"), C("param"),
      C("ensemble", "individual")}},

    {42, {C("generatingProcess"), C("statistics"), C("level"), C("composition", "chemical"), C("param")}},

    {43,
     {C("generatingProcess"), C("statistics"), C("level"), C("composition", "chemical"), C("param"),
      C("ensemble", "individual")}},

    {50, {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "aerosol"), C("param")}},

    {45,
     {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "aerosol"), C("param"),
      C("ensemble", "individual")}},

    {46, {C("generatingProcess"), C("statistics"), C("level"), C("composition", "aerosol"), C("param")}},

    {85,
     {C("generatingProcess"), C("statistics"), C("level"), C("composition", "aerosol"), C("param"),
      C("ensemble", "individual")}},

    {48, {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "aerosolOptical"), C("param")}},

    {49,
     {C("generatingProcess"), C("statistics"), C("level"), C("composition", "aerosolOptical"), C("param"),
      C("ensemble", "individual")}},

    {60, {C("generatingProcess"), C("referenceTime", "reforecast"), C("pointInTime"), C("level"), C("param")}},

    {61, {C("generatingProcess"), C("referenceTime", "reforecast"), C("statistics"), C("level"), C("param")}},

    {76, {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "chemicalSource"), C("param")}},

    {77,
     {C("generatingProcess"), C("pointInTime"), C("level"), C("composition", "chemicalSource"), C("param"),
      C("ensemble", "individual")}},

    {78, {C("generatingProcess"), C("statistics"), C("level"), C("composition", "chemicalSource"), C("param")}},

    {79,
     {C("generatingProcess"), C("statistics"), C("level"), C("composition", "chemicalSource"), C("param"),
      C("ensemble", "individual")}},

    {99, {C("generatingProcess"), C("pointInTime"), C("param"), C("wave", "spectra")}},

    {100, {C("generatingProcess"), C("pointInTime"), C("param"), C("wave", "spectra"), C("ensemble", "individual")}},

    {103, {C("generatingProcess"), C("pointInTime"), C("param"), C("wave", "period")}},

    {104, {C("generatingProcess"), C("pointInTime"), C("param"), C("wave", "period"), C("ensemble", "individual")}},

    {142, {C("generatingProcess"), C("pointInTime"), C("param"), C("ensemble", "perturbedParameters")}},

    {143, {C("generatingProcess"), C("pointInTime"), C("param"), C("ensemble", "randomPatterns")}}};

}  // namespace metkit::mars2grib::backend::sections::recipes

#pragma once


#include "metkit/mars2grib/backend/sections/recipes/RecipesCore.h"

namespace metkit::mars2grib::backend::sections::recipes {

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

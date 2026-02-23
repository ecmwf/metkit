//------------------------------------------------------------------------------
// Section 3 Recipes (runtime, static inline)
//------------------------------------------------------------------------------

#pragma once

// All elements needed to used the "dsl" objects to define recipes
#include "metkit/mars2grib/backend/sections/resolver/dsl.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::resolution::recipes::impl {

using namespace metkit::mars2grib::backend::concepts_;
using namespace metkit::mars2grib::backend::sections::resolver::dsl;

// clang-format off

//------------------------------------------------------------------------------
// Section 3 – Individual Recipes
//------------------------------------------------------------------------------

// Latitude / Longitude grid
inline const Recipe S3_R0 =
    make_recipe<0,
        Select<ShapeOfTheEarthConcept>,
        Select<RepresentationConcept,
            RepresentationType::Latlon>
    >();

// Reduced or Gaussian grid
inline const Recipe S3_R40 =
    make_recipe<40,
        Select<ShapeOfTheEarthConcept>,
        Select<RepresentationConcept,
            RepresentationType::ReducedGaussian,
            RepresentationType::RegularGaussian>
    >();

// Spectral representation
inline const Recipe S3_R50 =
    make_recipe<50,
        Select<RepresentationConcept,
            RepresentationType::SphericalHarmonics>
    >();

// General unstructured grid
inline const Recipe S3_R101 =
    make_recipe<101,
        Select<ShapeOfTheEarthConcept>,
        Select<RepresentationConcept,
            RepresentationType::GeneralUnstructured>
    >();

// HEALPix grid
inline const Recipe S3_R150 =
    make_recipe<150,
        Select<ShapeOfTheEarthConcept>,
        Select<RepresentationConcept,
            RepresentationType::Healpix>
    >();

//------------------------------------------------------------------------------
// Section 3 – Aggregated Recipes
//------------------------------------------------------------------------------

inline const Recipes Section3Recipes{ 3,
    std::vector<const Recipe*>{
        &S3_R0,
        &S3_R40,
        &S3_R50,
        &S3_R101,
        &S3_R150
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

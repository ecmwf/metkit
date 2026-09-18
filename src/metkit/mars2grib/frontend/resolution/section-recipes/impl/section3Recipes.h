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

inline const Recipe S3_R101_ORCA =
    make_recipe<101,
        Select<ShapeOfTheEarthConcept>,
        Select<RepresentationConcept,
            RepresentationType::Orca>
    >();

// HEALPix grid
inline const Recipe S3_R150 =
    make_recipe<150,
        Select<ShapeOfTheEarthConcept>,
        Select<RepresentationConcept,
            RepresentationType::Healpix>
    >();

// This is just meant to skip the section3 resolution in order to allow gridspec do be used after the encoder
inline const Recipe S3_R1000 =
    make_recipe<1000,
        Select<ShapeOfTheEarthConcept,
            ShapeOfTheEarthType::Dummy>,
        Select<RepresentationConcept,
            RepresentationType::Dummy>
    >();

// Same as above, but with prepping of the sample for spherical harmonics still enabled
inline const Recipe S3_R1001 =
    make_recipe<1001,
        Select<ShapeOfTheEarthConcept,
            ShapeOfTheEarthType::Dummy>,
        Select<RepresentationConcept,
            RepresentationType::DummySH>
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
        &S3_R101_ORCA,
        &S3_R150,
        &S3_R1000,
        &S3_R1001
    }
};

// clang-format on

}  // namespace metkit::mars2grib::frontend::resolution::recipes::impl

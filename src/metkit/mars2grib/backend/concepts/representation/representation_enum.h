#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// NAME OF THE CONCEPT
// ======================================================
inline constexpr std::string_view representationName{"representation"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class RepresentationType : std::size_t {
    Latlon = 0,
    RegularGaussian,
    ReducedGaussian,
    SphericalHarmonics,
    GeneralUnstructured,
    Healpix,
    Orca,
    Fesom,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using RepresentationList = ValueList<RepresentationType::Latlon, RepresentationType::RegularGaussian,
                                     RepresentationType::ReducedGaussian, RepresentationType::SphericalHarmonics,
                                     RepresentationType::GeneralUnstructured, RepresentationType::Healpix,
                                     RepresentationType::Orca, RepresentationType::Fesom, RepresentationType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <RepresentationType T>
constexpr std::string_view representationTypeName();

#define DEF(T, NAME)                                         \
    template <>                                              \
    constexpr std::string_view representationTypeName<T>() { \
        return NAME;                                         \
    }

DEF(RepresentationType::Latlon, "latlon");
DEF(RepresentationType::RegularGaussian, "regularGaussian");
DEF(RepresentationType::ReducedGaussian, "reducedGaussian");
DEF(RepresentationType::SphericalHarmonics, "sphericalHarmonics");
DEF(RepresentationType::GeneralUnstructured, "generalUnstructured");
DEF(RepresentationType::Healpix, "healpix");
DEF(RepresentationType::Orca, "orca");
DEF(RepresentationType::Fesom, "fesom");
DEF(RepresentationType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

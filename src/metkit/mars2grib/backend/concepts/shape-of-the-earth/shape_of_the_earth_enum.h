#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/backend/concepts/concept_core.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// NAME OF THE CONCEPT
// ======================================================
inline constexpr std::string_view shapeOfTheEarthName{"shapeOfTheEarth"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class ShapeOfTheEarthType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using ShapeOfTheEarthList = ValueList<ShapeOfTheEarthType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <ShapeOfTheEarthType T>
constexpr std::string_view shape_of_the_earthTypeName();

#define DEF(T, NAME)                                             \
    template <>                                                  \
    constexpr std::string_view shape_of_the_earthTypeName<T>() { \
        return NAME;                                             \
    }

DEF(ShapeOfTheEarthType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

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
inline constexpr std::string_view satelliteName{"satellite"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class SatelliteType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using SatelliteList = ValueList<SatelliteType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <SatelliteType T>
constexpr std::string_view satelliteTypeName();

#define DEF(T, NAME)                                    \
    template <>                                         \
    constexpr std::string_view satelliteTypeName<T>() { \
        return NAME;                                    \
    }

DEF(SatelliteType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

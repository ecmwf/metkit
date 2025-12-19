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
inline constexpr std::string_view destineName{"destine"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class DestineType : std::size_t {
    ClimateDT = 0,
    ExtremesDT
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using DestineList = ValueList<DestineType::ClimateDT, DestineType::ExtremesDT>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <DestineType T>
constexpr std::string_view destineTypeName();

#define DEF(T, NAME)                                  \
    template <>                                       \
    constexpr std::string_view destineTypeName<T>() { \
        return NAME;                                  \
    }

DEF(DestineType::ClimateDT, "climateDT");
DEF(DestineType::ExtremesDT, "extremesDT");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

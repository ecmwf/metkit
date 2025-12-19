#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/backend/concepts/concept_core.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// NAME OF THE CONCEPT
// ======================================================
inline constexpr std::string_view compositionName{"composition"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class CompositionType : std::size_t {
    Chem = 0,
    Aerosol,
    AerosolOptical,
    ChemicalSource,
    AerosolOpticalSource,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using CompositionList =
    ValueList<CompositionType::Chem, CompositionType::Aerosol, CompositionType::AerosolOptical,
              CompositionType::ChemicalSource, CompositionType::AerosolOpticalSource, CompositionType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <CompositionType T>
constexpr std::string_view compositionTypeName();

#define DEF(T, NAME)                                      \
    template <>                                           \
    constexpr std::string_view compositionTypeName<T>() { \
        return NAME;                                      \
    }

DEF(CompositionType::Chem, "chemical");
DEF(CompositionType::Aerosol, "aerosol");
DEF(CompositionType::AerosolOptical, "aerosolOptical");
DEF(CompositionType::ChemicalSource, "chemicalSource");
DEF(CompositionType::AerosolOpticalSource, "aerosolOpticalSource");
DEF(CompositionType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

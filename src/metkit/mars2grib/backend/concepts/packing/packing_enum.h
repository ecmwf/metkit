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
inline constexpr std::string_view packingName{"packing"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class PackingType : std::size_t {
    Simple = 0,
    Ccsds,
    SpectralComplex,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using PackingList =
    ValueList<PackingType::Simple, PackingType::Ccsds, PackingType::SpectralComplex, PackingType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <PackingType T>
constexpr std::string_view packingTypeName();

#define DEF(T, NAME)                                  \
    template <>                                       \
    constexpr std::string_view packingTypeName<T>() { \
        return NAME;                                  \
    }

DEF(PackingType::Simple, "simple");
DEF(PackingType::Ccsds, "ccsds");
DEF(PackingType::SpectralComplex, "spectral_complex");
DEF(PackingType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

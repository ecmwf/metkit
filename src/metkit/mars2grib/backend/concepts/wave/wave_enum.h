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
inline constexpr std::string_view waveName{"wave"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class WaveType : std::size_t {
    Spectra = 0,
    Period,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using WaveList = ValueList<WaveType::Spectra, WaveType::Period, WaveType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <WaveType T>
constexpr std::string_view waveTypeName();

#define DEF(T, NAME)                               \
    template <>                                    \
    constexpr std::string_view waveTypeName<T>() { \
        return NAME;                               \
    }

DEF(WaveType::Spectra, "spectra");
DEF(WaveType::Period, "period");
DEF(WaveType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

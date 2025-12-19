#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// NAME OF THE CONCEPT
// ======================================================
inline constexpr std::string_view generatingProcessName{"generatingProcess"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class GeneratingProcessType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using GeneratingProcessList = ValueList<GeneratingProcessType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <GeneratingProcessType T>
constexpr std::string_view generating_processTypeName();

#define DEF(T, NAME)                                             \
    template <>                                                  \
    constexpr std::string_view generating_processTypeName<T>() { \
        return NAME;                                             \
    }

DEF(GeneratingProcessType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

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
inline constexpr std::string_view referenceTimeName{"referenceTime"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class ReferenceTimeType : std::size_t {
    Standard   = 0,
    Reforecast = 1
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using ReferenceTimeList = ValueList<ReferenceTimeType::Standard, ReferenceTimeType::Reforecast>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <ReferenceTimeType T>
constexpr std::string_view referenceTimeTypeName();

#define DEF(T, NAME)                                        \
    template <>                                             \
    constexpr std::string_view referenceTimeTypeName<T>() { \
        return NAME;                                        \
    }
DEF(ReferenceTimeType::Standard, "standard");
DEF(ReferenceTimeType::Reforecast, "reforecast");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

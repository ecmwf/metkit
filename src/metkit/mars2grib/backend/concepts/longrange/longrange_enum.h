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
inline constexpr std::string_view longrangeName{"longrange"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class LongrangeType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using LongrangeList = ValueList<LongrangeType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <LongrangeType T>
constexpr std::string_view longrangeTypeName();

#define DEF(T, NAME)                                    \
    template <>                                         \
    constexpr std::string_view longrangeTypeName<T>() { \
        return NAME;                                    \
    }

DEF(LongrangeType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

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
inline constexpr std::string_view paramName{"param"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class ParamType : std::size_t {
    ParamId = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using ParamList = ValueList<ParamType::ParamId>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <ParamType T>
constexpr std::string_view paramTypeName();

#define DEF(T, NAME)                                \
    template <>                                     \
    constexpr std::string_view paramTypeName<T>() { \
        return NAME;                                \
    }

DEF(ParamType::ParamId, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

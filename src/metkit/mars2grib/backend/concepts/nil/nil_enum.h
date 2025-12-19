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
inline constexpr std::string_view nilName{"nil"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class NilType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using NilList = ValueList<NilType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <NilType T>
constexpr std::string_view nilTypeName();

#define DEF(T, NAME)                              \
    template <>                                   \
    constexpr std::string_view nilTypeName<T>() { \
        return NAME;                              \
    }

DEF(NilType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

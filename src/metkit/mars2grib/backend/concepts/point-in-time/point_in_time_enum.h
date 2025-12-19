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
inline constexpr std::string_view pointInTimeName{"pointInTime"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class PointInTimeType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using PointInTimeList = ValueList<PointInTimeType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <PointInTimeType T>
constexpr std::string_view pointInTimeTypeName();

#define DEF(T, NAME)                                      \
    template <>                                           \
    constexpr std::string_view pointInTimeTypeName<T>() { \
        return NAME;                                      \
    }

DEF(PointInTimeType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

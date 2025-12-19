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
inline constexpr std::string_view dataTypeName{"dataType"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class DataTypeType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using DataTypeList = ValueList<DataTypeType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <DataTypeType T>
constexpr std::string_view data_typeTypeName();

#define DEF(T, NAME)                                    \
    template <>                                         \
    constexpr std::string_view data_typeTypeName<T>() { \
        return NAME;                                    \
    }

DEF(DataTypeType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

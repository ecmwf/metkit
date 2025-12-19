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
inline constexpr std::string_view tablesName{"tables"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class TablesType : std::size_t {
    Custom = 0,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using TablesList = ValueList<TablesType::Custom, TablesType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <TablesType T>
constexpr std::string_view tablesTypeName();

#define DEF(T, NAME)                                 \
    template <>                                      \
    constexpr std::string_view tablesTypeName<T>() { \
        return NAME;                                 \
    }

DEF(TablesType::Custom, "custom");
DEF(TablesType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

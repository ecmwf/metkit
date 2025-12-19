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
inline constexpr std::string_view analysisName{"analysis"};

// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class AnalysisType : std::size_t {
    Default = 0
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using AnalysisList = ValueList<AnalysisType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <AnalysisType T>
constexpr std::string_view analysisTypeName();

#define DEF(T, NAME)                                   \
    template <>                                        \
    constexpr std::string_view analysisTypeName<T>() { \
        return NAME;                                   \
    }

DEF(AnalysisType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

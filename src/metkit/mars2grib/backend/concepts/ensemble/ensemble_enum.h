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
inline constexpr std::string_view ensembleName{"ensemble"};


// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class EnsembleType : std::size_t {
    Individual = 0,
    PerturbedParameters,
    RandomPatterns
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using EnsembleList =
    ValueList<EnsembleType::Individual, EnsembleType::PerturbedParameters, EnsembleType::RandomPatterns>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
template <EnsembleType T>
constexpr std::string_view ensembleTypeName();

#define DEF(T, NAME)                                   \
    template <>                                        \
    constexpr std::string_view ensembleTypeName<T>() { \
        return NAME;                                   \
    }

DEF(EnsembleType::Individual, "individual");
DEF(EnsembleType::PerturbedParameters, "perturbedParameters");
DEF(EnsembleType::RandomPatterns, "randomPatterns");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts

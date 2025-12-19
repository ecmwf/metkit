#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/backend/concepts/concept_core.h"

namespace metkit::mars2grib::backend {
// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
enum class EnsembleType : int {
    Individual = 0,
    Derived,
    PerturbedParameters,
    RandomPatterns,
    MeanUnweightedAll,
    MeanWeightedAll,
    StddevCluster,
    StddevClusterNorm,
    SpreadAll,
    LargeAnomalyIndex,
    MeanUnweightedCluster,
    Iqr,
    MinAll,
    MaxAll,
    VarianceAll,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using EnsembleList =
    ValueList<EnsembleType::Individual, EnsembleType::Derived, EnsembleType::PerturbedParameters,
              EnsembleType::RandomPatterns, EnsembleType::MeanUnweightedAll, EnsembleType::MeanWeightedAll,
              EnsembleType::StddevCluster, EnsembleType::StddevClusterNorm, EnsembleType::SpreadAll,
              EnsembleType::LargeAnomalyIndex, EnsembleType::MeanUnweightedCluster, EnsembleType::Iqr,
              EnsembleType::MinAll, EnsembleType::MaxAll, EnsembleType::VarianceAll, EnsembleType::Default>;

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
DEF(EnsembleType::Derived, "derived");
DEF(EnsembleType::PerturbedParameters, "perturbedParameters");
DEF(EnsembleType::RandomPatterns, "randomPatterns");
DEF(EnsembleType::MeanUnweightedAll, "meanUnweightedAll");
DEF(EnsembleType::MeanWeightedAll, "meanWeightedAll");
DEF(EnsembleType::StddevCluster, "stddevCluster");
DEF(EnsembleType::StddevClusterNorm, "stddevClusterNorm");
DEF(EnsembleType::SpreadAll, "spreadAll");
DEF(EnsembleType::LargeAnomalyIndex, "largeAnomalyIndex");
DEF(EnsembleType::MeanUnweightedCluster, "meanUnweightedCluster");
DEF(EnsembleType::Iqr, "iqr");
DEF(EnsembleType::MinAll, "minAll");
DEF(EnsembleType::MaxAll, "maxAll");
DEF(EnsembleType::VarianceAll, "varianceAll");
DEF(EnsembleType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend

/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file derivedEnum.h
/// @brief Definition of the `derived` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `derived` concept
/// used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`derivedName`)
/// - the exhaustive enumeration of supported derived variants (`DerivedType`)
/// - a compile-time typelist of all variants (`DerivedList`)
/// - a compile-time mapping from variant to string identifier
///
/// This file intentionally contains **no runtime logic** and **no encoding
/// behavior**. Its sole purpose is to provide compile-time metadata used by:
///
/// - the concept registry
/// - compile-time table generation
/// - logging and diagnostics
/// - static validation of concept variants
///
/// @note
/// This header is part of the **concept definition layer**.
/// Runtime behavior is implemented separately in the corresponding
/// `derived.h` / `derivedOp` implementation.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <cstdint>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <auto... Vals>
using ValueList = metkit::mars2grib::backend::compile_time_registry_engine::ValueList<Vals...>;

///
/// @brief Canonical name of the `derived` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the `derived` concept
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view derivedName{"derived"};


///
/// @brief Enumeration of all supported `derived` concept variants.
///
/// Each enumerator represents a specific derived product or statistical
/// transformation applied to ensemble or multi-field data.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @note
/// This enumeration includes both direct field selections and
/// post-processing/statistical aggregations.
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
///
enum class DerivedType : std::size_t {
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


///
/// @brief Compile-time list of all `derived` concept variants.
///
/// This typelist is used to:
/// - generate concept capability tables at compile time
/// - register all supported variants in the concept registry
/// - enable static iteration over variants without runtime overhead
///
/// @note
/// The order of this list must match the intended iteration order
/// for registry construction and diagnostics.
///
using DerivedList = ValueList<DerivedType::Individual, DerivedType::Derived, DerivedType::PerturbedParameters,
                              DerivedType::RandomPatterns, DerivedType::MeanUnweightedAll, DerivedType::MeanWeightedAll,
                              DerivedType::StddevCluster, DerivedType::StddevClusterNorm, DerivedType::SpreadAll,
                              DerivedType::LargeAnomalyIndex, DerivedType::MeanUnweightedCluster, DerivedType::Iqr,
                              DerivedType::MinAll, DerivedType::MaxAll, DerivedType::VarianceAll, DerivedType::Default>;


///
/// @brief Compile-time mapping from `DerivedType` to human-readable name.
///
/// This function returns the canonical string identifier associated
/// with a given derived variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Derived variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may
/// appear in logs, tests, and diagnostic output.
///
template <DerivedType T>
constexpr std::string_view derivedTypeName();

#define DEF(T, NAME)                                  \
    template <>                                       \
    constexpr std::string_view derivedTypeName<T>() { \
        return NAME;                                  \
    }

DEF(DerivedType::Individual, "individual");
DEF(DerivedType::Derived, "derived");
DEF(DerivedType::PerturbedParameters, "perturbedParameters");
DEF(DerivedType::RandomPatterns, "randomPatterns");
DEF(DerivedType::MeanUnweightedAll, "meanUnweightedAll");
DEF(DerivedType::MeanWeightedAll, "meanWeightedAll");
DEF(DerivedType::StddevCluster, "stddevCluster");
DEF(DerivedType::StddevClusterNorm, "stddevClusterNorm");
DEF(DerivedType::SpreadAll, "spreadAll");
DEF(DerivedType::LargeAnomalyIndex, "largeAnomalyIndex");
DEF(DerivedType::MeanUnweightedCluster, "meanUnweightedCluster");
DEF(DerivedType::Iqr, "iqr");
DEF(DerivedType::MinAll, "minAll");
DEF(DerivedType::MaxAll, "maxAll");
DEF(DerivedType::VarianceAll, "varianceAll");
DEF(DerivedType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_

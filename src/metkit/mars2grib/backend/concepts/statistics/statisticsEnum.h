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
/// @file statisticsEnum.h
/// @brief Definition of the `statistics` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `statistics` concept
/// used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`statisticsName`)
/// - the exhaustive enumeration of supported statistical processing variants (`StatisticsType`)
/// - a compile-time typelist of all variants (`StatisticsList`)
/// - a compile-time mapping from variant to string identifier
/// - a compile-time mapping from variant to GRIB `typeOfStatisticalProcessing` code
///
/// This file intentionally contains **no runtime logic** and **no encoding
/// behavior**. Its sole purpose is to provide compile-time metadata used by:
///
/// - the concept registry
/// - compile-time table generation
/// - logging and diagnostics
/// - static validation of concept variants
/// - static mapping to GRIB statistical processing identifiers
///
/// @note
/// This header is part of the **concept definition layer**.
/// Runtime behavior is implemented separately in the corresponding
/// `statistics.h` / `statisticsOp` implementation.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <cstdint>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"

namespace metkit::mars2grib::backend::concepts_ {

template <auto... Vals>
using ValueList = metkit::mars2grib::backend::compile_time_registry_engine::ValueList<Vals...>;


///
/// @brief Canonical name of the `statistics` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the `statistics` concept
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view statisticsName{"statistics"};


///
/// @brief Enumeration of all supported `statistics` concept variants.
///
/// Each enumerator represents a distinct statistical processing operation
/// applied to time ranges, ensembles, or aggregated datasets, as defined
/// by GRIB conventions and ECMWF usage.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @note
/// This enumeration includes both standard GRIB statistical operators
/// and extended or ECMWF-specific processing types.
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
///
enum class StatisticsType : std::size_t {
    Average = 0,
    Accumulation,
    Maximum,
    Minimum,
    DifferenceFromStart,
    RootMeanSquare,
    StandardDeviation,
    Covariance,
    DifferenceFromEnd,
    Ratio,
    StandardizedAnomaly,
    Summation,
    ReturnPeriod,
    Median,
    Severity,
    Mode,
    IndexProcessing,
    Default
};


///
/// @brief Compile-time list of all `statistics` concept variants.
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
using StatisticsList =
    ValueList<StatisticsType::Average, StatisticsType::Accumulation, StatisticsType::Maximum, StatisticsType::Minimum,
              StatisticsType::DifferenceFromStart, StatisticsType::RootMeanSquare, StatisticsType::StandardDeviation,
              StatisticsType::Covariance, StatisticsType::DifferenceFromEnd, StatisticsType::Ratio,
              StatisticsType::StandardizedAnomaly, StatisticsType::Summation, StatisticsType::ReturnPeriod,
              StatisticsType::Median, StatisticsType::Severity, StatisticsType::Mode, StatisticsType::IndexProcessing,
              StatisticsType::Default>;


///
/// @brief Compile-time mapping from `StatisticsType` to human-readable name.
///
/// This function returns the canonical string identifier associated
/// with a given statistical processing variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Statistics variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may
/// appear in logs, tests, and diagnostic output.
///
template <StatisticsType T>
constexpr std::string_view statisticsTypeName();

#define DEF(T, NAME)                                     \
    template <>                                          \
    constexpr std::string_view statisticsTypeName<T>() { \
        return NAME;                                     \
    }

DEF(StatisticsType::Average, "average");
DEF(StatisticsType::Accumulation, "accumulation");
DEF(StatisticsType::Maximum, "maximum");
DEF(StatisticsType::Minimum, "minimum");
DEF(StatisticsType::DifferenceFromStart, "differenceFromStart");
DEF(StatisticsType::RootMeanSquare, "rootMeanSquare");
DEF(StatisticsType::StandardDeviation, "standardDeviation");
DEF(StatisticsType::Covariance, "covariance");
DEF(StatisticsType::DifferenceFromEnd, "differenceFromEnd");
DEF(StatisticsType::Ratio, "ratio");
DEF(StatisticsType::StandardizedAnomaly, "standardizedAnomaly");
DEF(StatisticsType::Summation, "summation");
DEF(StatisticsType::ReturnPeriod, "returnPeriod");
DEF(StatisticsType::Median, "median");
DEF(StatisticsType::Severity, "severity");
DEF(StatisticsType::Mode, "mode");
DEF(StatisticsType::IndexProcessing, "indexProcessing");
DEF(StatisticsType::Default, "default");

#undef DEF


///
/// @brief Compile-time mapping from `StatisticsType` to GRIB `typeOfStatisticalProcessing` code.
///
/// This function returns the numeric GRIB code associated with a given
/// statistical processing variant, as defined by GRIB tables and
/// ECMWF extensions.
///
/// The returned value is used during encoding to populate the
/// `typeOfStatisticalProcessing` key in GRIB messages.
///
/// @tparam T Statistics variant
/// @return GRIB code corresponding to the variant
///
/// @note
/// The returned values must remain stable across releases to ensure
/// GRIB compatibility.
///
template <StatisticsType T>
constexpr long typeOfStatisticalProcessing();

#define DEF(T, NAME)                                  \
    template <>                                       \
    constexpr long typeOfStatisticalProcessing<T>() { \
        return NAME;                                  \
    }

DEF(StatisticsType::Average, 0);
DEF(StatisticsType::Accumulation, 1);
DEF(StatisticsType::Maximum, 2);
DEF(StatisticsType::Minimum, 3);
DEF(StatisticsType::DifferenceFromStart, 4);
DEF(StatisticsType::RootMeanSquare, 5);
DEF(StatisticsType::StandardDeviation, 6);
DEF(StatisticsType::Covariance, 7);
DEF(StatisticsType::DifferenceFromEnd, 8);
DEF(StatisticsType::Ratio, 9);
DEF(StatisticsType::StandardizedAnomaly, 10);
DEF(StatisticsType::Summation, 11);
DEF(StatisticsType::ReturnPeriod, 12);
DEF(StatisticsType::Median, 13);
DEF(StatisticsType::Severity, 100);
DEF(StatisticsType::Mode, 101);
DEF(StatisticsType::IndexProcessing, 102);
DEF(StatisticsType::Default, 255);

#undef DEF


}  // namespace metkit::mars2grib::backend::concepts_

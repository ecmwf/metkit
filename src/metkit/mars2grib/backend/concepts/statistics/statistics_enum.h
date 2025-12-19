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
inline constexpr std::string_view statisticsName{"statistics"};


// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
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

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using StatisticsList =
    ValueList<StatisticsType::Average, StatisticsType::Accumulation, StatisticsType::Maximum, StatisticsType::Minimum,
              StatisticsType::DifferenceFromStart, StatisticsType::RootMeanSquare, StatisticsType::StandardDeviation,
              StatisticsType::Covariance, StatisticsType::DifferenceFromEnd, StatisticsType::Ratio,
              StatisticsType::StandardizedAnomaly, StatisticsType::Summation, StatisticsType::ReturnPeriod,
              StatisticsType::Median, StatisticsType::Severity, StatisticsType::Mode, StatisticsType::IndexProcessing,
              StatisticsType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
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


// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
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


}  // namespace metkit::mars2grib::backend::cnpts

// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file focalStatisticsEnum.h
/// @brief Definition of the `focalStatistics` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `focalStatistics`
/// concept used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`focalStatisticsName`)
/// - the enumeration of supported focal statistics variants (`FocalStatisticsType`)
/// - a compile-time typelist of all variants (`FocalStatisticsList`)
/// - a compile-time mapping from variant to string identifier
///
/// The concept captures the spatio-temporal window (vicinity) over which a
/// product is computed (e.g. the strike probabilities of PDT 4.121/4.122).
///
/// This file intentionally contains **no runtime logic** and **no encoding
/// behavior**.
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
/// @brief Canonical name of the `focalStatistics` concept.
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view focalStatisticsName{"focalStatistics"};


///
/// @brief Enumeration of all supported `focalStatistics` concept variants.
///
/// - `Default`: spatio-temporal vicinity (focal statistics) window
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
///
enum class FocalStatisticsType : std::size_t {
    Default = 0
};


///
/// @brief Compile-time list of all `focalStatistics` concept variants.
///
using FocalStatisticsList = ValueList<FocalStatisticsType::Default>;


///
/// @brief Compile-time mapping from `FocalStatisticsType` to human-readable name.
///
/// @tparam T Focal statistics variant
/// @return String view identifying the variant
///
template <FocalStatisticsType T>
constexpr std::string_view focalStatisticsTypeName();

#define DEF(T, NAME)                                          \
    template <>                                               \
    constexpr std::string_view focalStatisticsTypeName<T>() { \
        return NAME;                                          \
    }

DEF(FocalStatisticsType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_

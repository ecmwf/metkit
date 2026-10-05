/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>
#include <string_view>

#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"

namespace metkit::mars2grib::backend::concepts_ {

inline constexpr std::string_view quantileName{"quantile"};

enum class QuantileType : std::size_t {
    Default = 0,
};

using QuantileList = compile_time_registry_engine::ValueList<QuantileType::Default>;

template <QuantileType T>
constexpr std::string_view quantileTypeName();

template <>
constexpr std::string_view quantileTypeName<QuantileType::Default>() {
    return "default";
}


}  // namespace metkit::mars2grib::backend::concepts_

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

/// Post-processed products: the `postproc` block of PDT 4.89/4.90/4.133/4.135
/// (`inputProcessIdentifier`, `inputOriginatingCentre`, `typeOfPostProcessing`).
inline constexpr std::string_view postProcessingName{"postProcessing"};

enum class PostProcessingType : std::size_t {
    Default = 0,
};

using PostProcessingList = compile_time_registry_engine::ValueList<PostProcessingType::Default>;

template <PostProcessingType T>
constexpr std::string_view postProcessingTypeName();

template <>
constexpr std::string_view postProcessingTypeName<PostProcessingType::Default>() {
    return "default";
}

}  // namespace metkit::mars2grib::backend::concepts_

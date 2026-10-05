/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/compile-time-registry-engine/RegisterEntryDescriptor.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/post-processing/postProcessingEncoding.h"
#include "metkit/mars2grib/backend/concepts/post-processing/postProcessingEnum.h"
#include "metkit/mars2grib/backend/concepts/post-processing/postProcessingMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

using namespace metkit::mars2grib::backend::compile_time_registry_engine;

struct PostProcessingConcept : RegisterEntryDescriptor<PostProcessingType, PostProcessingList> {
    static constexpr std::string_view entryName() { return postProcessingName; }

    template <PostProcessingType T>
    static constexpr std::string_view variantName() {
        return postProcessingTypeName<T>();
    }

    template <std::size_t Capability, std::size_t Stage, std::size_t Sec, PostProcessingType Variant, class MarsDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> phaseCallbacks() {
        if constexpr (Capability == 0 && postProcessingApplicable<Stage, Sec, Variant>()) {
            return &PostProcessingOp<Stage, Sec, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }
    }

    template <std::size_t Capability, PostProcessingType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> variantCallbacks() {
        return nullptr;
    }

    template <std::size_t Capability, class MarsDict_t, class OptDict_t>
    static constexpr Fm<MarsDict_t, OptDict_t> entryCallbacks() {
        if constexpr (Capability == 0) {
            return &postProcessingMatcher<MarsDict_t, OptDict_t>;
        }
        else {
            return nullptr;
        }
    }
};

}  // namespace metkit::mars2grib::backend::concepts_

#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/param/param_encoding.h"
#include "metkit/mars2grib/backend/concepts/param/param_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct ParamConceptInfo {
    static constexpr const char* name = "param";

    template <std::size_t Stage, std::size_t Sec, ParamType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
              class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (paramApplicable<Stage, Sec, Variant>()) {
            return &ParamOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(paramTypeName<static_cast<ParamType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

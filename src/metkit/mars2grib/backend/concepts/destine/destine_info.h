#pragma once

#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/destine/destine_encoding.h"
#include "metkit/mars2grib/backend/concepts/destine/destine_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct DestineConceptInfo {
    static constexpr const char* name = destineName.data();

    template <std::size_t Stage, std::size_t Sec, DestineType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (destineApplicable<Stage, Sec, Variant>()) {
            return &DestineOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(destineTypeName<static_cast<DestineType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

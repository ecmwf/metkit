#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/origin/origin_encoding.h"
#include "metkit/mars2grib/backend/concepts/origin/origin_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct OriginConceptInfo {
    static constexpr const char* name = originName.data();

    template <std::size_t Stage, std::size_t Sec, OriginType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (originApplicable<Stage, Sec, Variant>()) {
            return &OriginOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(originTypeName<static_cast<OriginType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

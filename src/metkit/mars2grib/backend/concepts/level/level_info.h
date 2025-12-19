#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/level/level_encoding.h"
#include "metkit/mars2grib/backend/concepts/level/level_enum.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct LevelConceptInfo {
    static constexpr const char* name = levelName.data();

    template <std::size_t Stage, std::size_t Sec, LevelType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
              class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (levelApplicable<Stage, Sec, Variant>()) {
            return &LevelOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(levelTypeName<static_cast<LevelType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

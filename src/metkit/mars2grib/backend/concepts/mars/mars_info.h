#pragma once

#include <cstdint>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/mars/mars_encoding.h"
#include "metkit/mars2grib/backend/concepts/mars/mars_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct MarsConceptInfo {
    static constexpr const char* name = marsName.data();

    template <std::size_t Stage, std::size_t Sec, MarsType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
              class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (marsApplicable<Stage, Sec, Variant>()) {
            return &MarsOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(marsTypeName<static_cast<MarsType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

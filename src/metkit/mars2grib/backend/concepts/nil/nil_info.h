#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/nil/nil_encoding.h"
#include "metkit/mars2grib/backend/concepts/nil/nil_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct NilConceptInfo {
    static constexpr const char* name = nilName.data();

    template <std::size_t Stage, std::size_t Sec, NilType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
              class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (nilApplicable<Stage, Sec, Variant>()) {
            return &NilOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(nilTypeName<static_cast<NilType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

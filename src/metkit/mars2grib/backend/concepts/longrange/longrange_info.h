#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrange_encoding.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrange_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct LongrangeConceptInfo {
    static constexpr const char* name = longrangeName.data();

    template <std::size_t Stage, std::size_t Sec, LongrangeType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (longrangeApplicable<Stage, Sec, Variant>()) {
            return &LongrangeOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(longrangeTypeName<static_cast<LongrangeType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

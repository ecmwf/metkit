#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/packing/packing_encoding.h"
#include "metkit/mars2grib/backend/concepts/packing/packing_enum.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct PackingConceptInfo {
    static constexpr const char* name = "packing";

    template <std::size_t Stage, std::size_t Section, PackingType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (packingApplicable<Stage, Section, Variant>()) {
            return &PackingOp<Stage, Section, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Remove compiler warning
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(packingTypeName<static_cast<PackingType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

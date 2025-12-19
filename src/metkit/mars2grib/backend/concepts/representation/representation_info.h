#pragma once

#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/representation/representation_encoding.h"
#include "metkit/mars2grib/backend/concepts/representation/representation_enum.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct RepresentationConceptInfo {
    static constexpr const char* name = representationName.data();

    template <std::size_t Stage, std::size_t Section, RepresentationType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (representationApplicable<Stage, Section, Variant>()) {
            return &RepresentationOp<Stage, Section, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(representationTypeName<static_cast<RepresentationType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

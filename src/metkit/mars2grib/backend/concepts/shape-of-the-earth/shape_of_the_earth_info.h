#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shape_of_the_earth_encoding.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shape_of_the_earth_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct ShapeOfTheEarthConceptInfo {
    static constexpr const char* name = shapeOfTheEarthName.data();

    template <std::size_t Stage, std::size_t Section, ShapeOfTheEarthType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (shape_of_the_earthApplicable<Stage, Section, Variant>()) {
            return &ShapeOfTheEarthOp<Stage, Section, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Remove compiler warning
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(shape_of_the_earthTypeName<static_cast<ShapeOfTheEarthType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

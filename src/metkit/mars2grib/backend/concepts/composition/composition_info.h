#pragma once

#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/composition/composition_encoding.h"
#include "metkit/mars2grib/backend/concepts/composition/composition_enum.h"
#include "metkit/mars2grib/backend/concepts/concept_core.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct CompositionConceptInfo {
    static constexpr const char* name = compositionName.data();

    template <std::size_t Stage, std::size_t Sec, CompositionType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (compositionApplicable<Stage, Sec, Variant>()) {
            return &CompositionOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(compositionTypeName<static_cast<CompositionType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts//concept_core.h"
#include "metkit/mars2grib/backend/concepts//ensemble/ensemble_encoding.h"
#include "metkit/mars2grib/backend/concepts//ensemble/ensemble_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct EnsembleConceptInfo {
    static constexpr const char* name = ensembleName.data();

    template <std::size_t Stage, std::size_t Sec, EnsembleType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (ensembleApplicable<Stage, Sec, Variant>()) {
            return &EnsembleOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(ensembleTypeName<static_cast<EnsembleType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

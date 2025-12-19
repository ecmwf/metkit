#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/analysis/analysis_encoding.h"
#include "metkit/mars2grib/backend/concepts/analysis/analysis_enum.h"
#include "metkit/mars2grib/backend/concepts/concept_core.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct AnalysisConceptInfo {
    static constexpr const char* name = analysisName.data();

    template <std::size_t Stage, std::size_t Sec, AnalysisType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (analysisApplicable<Stage, Sec, Variant>()) {
            return &AnalysisOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(analysisTypeName<static_cast<AnalysisType>(Variant)>());
    }
};


}  // namespace metkit::mars2grib::backend::cnpts

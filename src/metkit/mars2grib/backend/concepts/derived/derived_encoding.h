#pragma once

#include <iostream>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/derived/derived_enum.h"

namespace metkit::mars2grib::backend {
// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
constexpr bool ensembleApplicable(int Stage, int Section, EnsembleType Variant) {
    return true;
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <int Stage, int Section, EnsembleType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
uint8_t EnsembleOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                   OutDict_t& out) {
    LOG_DEBUG_LIB(LibMetkit) << "[Concept Ensemble] Op called: "
                             << "Stage=" << Stage << ", Section=" << Section
                             << ", Variant=" << std::string(ensembleTypeName<Variant>()) << std::endl;
    return 0;
}
}  // namespace metkit::mars2grib::backend

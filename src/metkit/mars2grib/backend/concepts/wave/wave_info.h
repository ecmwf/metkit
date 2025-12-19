#pragma once

#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/wave/wave_encoding.h"
#include "metkit/mars2grib/backend/concepts/wave/wave_enum.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct WaveConceptInfo {
    static constexpr const char* name = waveName.data();

    template <std::size_t Stage, std::size_t Sec, WaveType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
              class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (waveApplicable<Stage, Sec, Variant>()) {
            return &WaveOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(waveTypeName<static_cast<WaveType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

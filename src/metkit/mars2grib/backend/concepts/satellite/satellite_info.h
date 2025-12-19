#pragma once

#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/satellite/satellite_encoding.h"
#include "metkit/mars2grib/backend/concepts/satellite/satellite_enum.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct SatelliteConceptInfo {
    static constexpr const char* name = satelliteName.data();

    template <std::size_t Stage, std::size_t Section, SatelliteType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (satelliteApplicable<Stage, Section, Variant>()) {
            return &SatelliteOp<Stage, Section, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(satelliteTypeName<static_cast<SatelliteType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

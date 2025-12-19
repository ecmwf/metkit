#pragma once

#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/point-in-time/point_in_time_encoding.h"
#include "metkit/mars2grib/backend/concepts/point-in-time/point_in_time_enum.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct PointInTimeConceptInfo {
    static constexpr const char* name = pointInTimeName.data();

    template <std::size_t Stage, std::size_t Sec, PointInTimeType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (pointInTimeApplicable<Stage, Sec, Variant>()) {
            return &PointInTimeOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(pointInTimeTypeName<static_cast<PointInTimeType>(Variant)>());
    }
};


}  // namespace metkit::mars2grib::backend::cnpts

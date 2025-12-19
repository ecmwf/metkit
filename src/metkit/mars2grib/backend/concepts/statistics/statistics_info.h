#pragma once

#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/statistics/statistics_encoding.h"
#include "metkit/mars2grib/backend/concepts/statistics/statistics_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct StatisticsConceptInfo {
    static constexpr const char* name = statisticsName.data();

    template <std::size_t Stage, std::size_t Sec, StatisticsType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (statisticsApplicable<Stage, Sec, Variant>()) {
            return &StatisticsOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(statisticsTypeName<static_cast<StatisticsType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

#pragma once

#include <string>
#include <string_view>

#include "metkit/config/LibMetkit.h"

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/tables/tables_encoding.h"
#include "metkit/mars2grib/backend/concepts/tables/tables_enum.h"

namespace metkit::mars2grib::backend::cnpts {


// ======================================================
// ConceptInfo
// ======================================================
struct TablesConceptInfo {
    static constexpr const char* name = tablesName.data();

    template <std::size_t Stage, std::size_t Sec, TablesType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (tablesApplicable<Stage, Sec, Variant>()) {
            return &TablesOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(tablesTypeName<static_cast<TablesType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

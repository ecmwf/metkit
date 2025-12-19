#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/data-type/data_type_encoding.h"
#include "metkit/mars2grib/backend/concepts/data-type/data_type_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// ConceptInfo
// ======================================================
struct DataTypeConceptInfo {
    static constexpr const char* name = dataTypeName.data();

    template <std::size_t Stage, std::size_t Sec, DataTypeType Variant, class MarsDict_t, class GeoDict_t,
              class ParDict_t, class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (data_typeApplicable<Stage, Sec, Variant>()) {
            return &DataTypeOp<Stage, Sec, Variant, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(data_typeTypeName<static_cast<DataTypeType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::cnpts

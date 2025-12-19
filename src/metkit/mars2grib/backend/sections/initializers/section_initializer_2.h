#pragma once

#include <iostream>

#include "metkit/mars2grib/backend/sections/initializers/section_initializer_core.h"

#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::initializers {

template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void allocateTemplateNumber2(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                             OutDict_t& out) {
    // Dictionary traits
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Allocate a local use section (I don't check if it exists already, just overwrite)
        set_or_throw<long>(out, "setLocalDefinition", 1);

        // Set the local definition number based on template number
        // Special cases for some template numbers related to DestinE
        // These template numbers are not official and not defined in eccodes,
        // they are used in the encoders as virtual local definition sections to
        // allow the same behaviour as official template numbers.
        if constexpr (TemplateNumber == 1001) {
            // Just set the minimal section2
            set_or_throw<long>(out, "localDefinitionNumber", 1L);
            // Add additional values for DestinE
            set_or_throw<long>(out, "productionStatusOfProcessedData", 12L);
            // Add additional values for ClimateDT
            set_or_throw<std::string>(out, "dataset", "climate-dt");
        }
        else if constexpr (TemplateNumber == 1002) {
            // Just set the minimal section2
            set_or_throw<long>(out, "localDefinitionNumber", 1L);
            // Add additional values for DestinE
            set_or_throw<long>(out, "productionStatusOfProcessedData", 12L);
            // Add additional values for ExtremesDT
            set_or_throw<std::string>(out, "dataset", "extremes-dt");
        }
        else {
            set_or_throw<long>(out, "localDefinitionNumber", TemplateNumber);
        }

        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error preparing section 2 with template number", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::sections::initializers

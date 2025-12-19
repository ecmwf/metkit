#pragma once

#include <iostream>

#include "metkit/mars2grib/backend/sections/initializers/section_initializer_core.h"

#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::initializers {

template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void allocateTemplateNumber5(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                             OutDict_t& out) {
    // Dictionary traits
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        long pdt = static_cast<long>(TemplateNumber);
        set_or_throw<long>(out, "dataRepresentationTemplateNumber", pdt);

        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error preparing section 5 with template number", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::sections::initializers

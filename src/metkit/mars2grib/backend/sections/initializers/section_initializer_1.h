#pragma once

#include <iostream>
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_core.h"

namespace metkit::mars2grib::backend::sections::initializers {

template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void allocateTemplateNumber1(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                             OutDict_t& out) {
    // No-op for section 0
}

}  // namespace metkit::mars2grib::backend::sections::initializers

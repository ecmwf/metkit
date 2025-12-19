#pragma once

#include <iostream>
#include <vector>

#include "metkit/mars2grib/backend/sections/initializers/section_initializer_core.h"

#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::initializers {


template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void allocateTemplateNumber3(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                             OutDict_t& out) {
    // Dictionary traits
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Eventually prepare the sample the sample if special handling is needed
        if constexpr (TemplateNumber == 50) {
            // https://confluence.ecmwf.int/display/ECC/ecCodes+developer+FAQ+-+GRIB#ecCodesdeveloperFAQGRIB-GRIB:HowcanIconvertthesampleGRIB2.tmpltosphericalharmonics?
            set_or_throw<long>(out, "numberOfDataPoints", 6L);
            set_or_throw<long>(out, "numberOfValues", 6L);
            set_or_throw<long>(out, "bitsPerValue", 16L);
            set_or_throw<long>(out, "typeOfFirstFixedSurface", 105L);
            set_or_throw<std::vector<double>>(out, "values", std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
            set_or_throw<long>(out, "scaleFactorOfFirstFixedSurface", 0L);
            set_or_throw<long>(out, "scaledValueOfFirstFixedSurface", 0L);
            set_or_throw<long>(out, "gridDefinitionTemplateNumber", 50L);
            set_or_throw<long>(out, "J", 1L);
            set_or_throw<long>(out, "K", 1L);
            set_or_throw<long>(out, "M", 1L);
            set_or_throw<long>(out, "spectralType", 1L);
            set_or_throw<long>(out, "spectralMode", 1L);
            set_or_throw<long>(out, "numberOfOctectsForNumberOfPoints", 0L);
            set_or_throw<long>(out, "interpretationOfNumberOfPoints", 0L);
            set_or_throw<long>(out, "dataRepresentationTemplateNumber", 51L);
        }
        else {
            long drt = static_cast<long>(TemplateNumber);
            set_or_throw<long>(out, "gridDefinitionTemplateNumber", drt);
        }

        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error preparing section 3 with template number", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::sections::initializers

/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file section4Initializer.h
 * @brief Initializer for GRIB Section 4 (Product Definition Section).
 *
 * This header defines a **section initializer** for GRIB **Section 4**,
 * responsible for selecting and setting the *Product Definition Template*
 * (PDT) number.
 *
 * Section 4 describes the scientific meaning of the data (forecast,
 * analysis, ensemble, statistics, etc.). In the mars2grib backend,
 * the template number is resolved by the concept layer and passed
 * unchanged to the GRIB output dictionary by this initializer.
 *
 * This initializer performs a single, well-defined mutation of the
 * output dictionary and relies on strict error checking and structured
 * exception propagation.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once

#include "metkit/mars2grib/backend/sections/initializers/sectionInitializerCore.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::initializers {

/**
 * @brief Initializer for GRIB Section 4 (Product Definition Section).
 *
 * This function configures Section 4 by setting the
 * **Product Definition Template Number** corresponding to the
 * resolved product type.
 *
 * No additional preprocessing is performed at this stage; all
 * concept-specific logic affecting Section 4 is handled elsewhere
 * in the encoding pipeline.
 *
 * @tparam SectionNumber   GRIB section number (expected to be 4)
 * @tparam TemplateNumber  Product Definition Template number
 * @tparam MarsDict_t      Type of the MARS dictionary
 * @tparam ParDict_t       Type of the parameter dictionary
 * @tparam OptDict_t       Type of the options dictionary
 * @tparam OutDict_t       Type of the output GRIB dictionary
 *
 * @param mars Read-only MARS dictionary
 * @param par  Read-only parameter dictionary
 * @param opt  Read-only options dictionary
 * @param out  Output GRIB dictionary to be populated
 *
 * @throws Mars2GribGenericException
 *         If setting the product definition template number fails.
 */
template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void allocateTemplateNumber4(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    // Dictionary traits
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        long pdt = static_cast<long>(TemplateNumber);
        set_or_throw<long>(out, "productDefinitionTemplateNumber", pdt);

        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error preparing section 4 with template number", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::sections::initializers

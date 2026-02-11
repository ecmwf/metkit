/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file section5Initializer.h
/// @brief Initializer for GRIB Section 5 (Data Representation Section).
///
/// This header defines a **section initializer** for GRIB **Section 5**,
/// responsible for selecting the *Data Representation Template* (DRT)
/// used to encode the data values.
///
/// Section 5 controls how the field values are packed (e.g. simple packing,
/// complex packing, JPEG, PNG, spectral, etc.). In the mars2grib backend,
/// the concrete template number is resolved by the concept layer and passed
/// directly to the GRIB output dictionary by this initializer.
///
/// This initializer performs a single, well-defined mutation of the output
/// dictionary and relies on strict error checking and structured exception
/// propagation.
///
/// @ingroup mars2grib_backend_sections
///
#pragma once

#include "metkit/mars2grib/backend/sections/initializers/sectionInitializerCore.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::sections::initializers {

///
/// @brief Initializer for GRIB Section 5 (Data Representation Section).
///
/// This function configures Section 5 by setting the
/// **Data Representation Template Number** corresponding to the
/// selected packing or representation method.
///
/// No additional preprocessing is performed at this stage; all
/// concept-specific logic affecting Section 5 is handled elsewhere
/// in the encoding pipeline.
///
/// @tparam SectionNumber   GRIB section number (expected to be 5)
/// @tparam TemplateNumber  Data Representation Template number
/// @tparam MarsDict_t      Type of the MARS dictionary
/// @tparam ParDict_t       Type of the parameter dictionary
/// @tparam OptDict_t       Type of the options dictionary
/// @tparam OutDict_t       Type of the output GRIB dictionary
///
/// @param mars Read-only MARS dictionary
/// @param par  Read-only parameter dictionary
/// @param opt  Read-only options dictionary
/// @param out  Output GRIB dictionary to be populated
///
/// @throws Mars2GribGenericException
/// If setting the data representation template number fails.
///
template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void allocateTemplateNumber5(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
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
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::sections::initializers

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
 * @file section3Initializer.h
 * @brief Initializer for GRIB Section 3 (Grid Definition Section).
 *
 * This header defines a **section initializer** for GRIB **Section 3**,
 * responsible for configuring the *Grid Definition Section* according to
 * the selected grid definition template.
 *
 * The initializer supports:
 * - standard grid definition templates, mapped directly from the template number
 * - special-case handling for selected templates that require explicit
 *   preconditioning of the GRIB handle prior to encoding
 *
 * In particular, Template **50** (spectral representation) requires
 * explicit initialization of several keys (values, truncation parameters,
 * spectral mode, etc.) to satisfy ecCodes constraints and enable correct
 * conversion to spherical harmonics.
 *
 * All dictionary mutations are performed via checked dictionary traits,
 * and failures are wrapped in a mars2grib-specific exception with
 * preserved exception nesting.
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once

// System includes
#include <vector>

#include "metkit/mars2grib/backend/sections/initializers/sectionInitializerCore.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::initializers {

/**
 * @brief Initializer for GRIB Section 3 (Grid Definition Section).
 *
 * This function prepares Section 3 by selecting and configuring the
 * appropriate *Grid Definition Template*.
 *
 * Behaviour depends on the template number:
 * - **Template 50** (spectral grid): requires explicit initialization of
 *   grid size, spectral truncation parameters, representation mode, and
 *   placeholder values, following ecCodes recommendations
 * - **All other templates**: the grid definition template number is set
 *   directly with no additional preprocessing
 *
 * @tparam SectionNumber   GRIB section number (expected to be 3)
 * @tparam TemplateNumber  Grid definition template number
 * @tparam MarsDict_t      Type of the MARS dictionary
 * @tparam GeoDict_t       Type of the geometry dictionary
 * @tparam ParDict_t       Type of the parameter dictionary
 * @tparam OptDict_t       Type of the options dictionary
 * @tparam OutDict_t       Type of the output GRIB dictionary
 *
 * @param mars Read-only MARS dictionary
 * @param geo  Read-only geometry dictionary
 * @param par  Read-only parameter dictionary
 * @param opt  Read-only options dictionary
 * @param out  Output GRIB dictionary to be populated
 *
 * @throws Mars2GribGenericException
 *         If any dictionary operation fails while preparing Section 3.
 *
 * @note
 * The special handling for Template 50 follows ecCodes guidance for
 * spectral GRIB messages:
 * https://confluence.ecmwf.int/display/ECC/ecCodes+developer+FAQ+-+GRIB#ecCodesdeveloperFAQGRIB-GRIB:HowcanIconvertthesampleGRIB2.tmpltosphericalharmonics?
 */
template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void allocateTemplateNumber3(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                             OutDict_t& out) {
    // Dictionary traits
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Special handling for spectral grids (Template 50)
        if constexpr (TemplateNumber == 50) {
            // Precondition GRIB handle for spectral representation
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
            // Standard grid definition template
            long drt = static_cast<long>(TemplateNumber);
            set_or_throw<long>(out, "gridDefinitionTemplateNumber", drt);
            set_or_throw<long>(out, "resolutionAndComponentFlags", 0L);
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

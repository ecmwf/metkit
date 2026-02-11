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
/// @file section2Initializer.h
/// @brief Initializer for GRIB Section 2 (Local Use Section).
///
/// This header defines a **section initializer** for GRIB **Section 2**,
/// responsible for allocating and populating the *Local Use Section*
/// based on the resolved template number.
///
/// Section 2 is used to encode local or centre-specific extensions that
/// are not part of the official GRIB specification. In the mars2grib
/// backend, this initializer supports both:
///
/// - standard local definition numbers
/// - *virtual* template numbers used internally by the encoder
///
/// In particular, this initializer contains special handling for
/// DestinE-related virtual template numbers that are mapped onto
/// valid local definition sections with additional metadata.
///
/// This file performs controlled mutation of the output GRIB dictionary
/// and includes structured exception handling to ensure consistent
/// error propagation across section initialization boundaries.
///
/// @ingroup mars2grib_backend_sections
///
#pragma once

#include "metkit/mars2grib/backend/sections/initializers/sectionInitializerCore.h"

namespace metkit::mars2grib::backend::sections::initializers {

///
/// @brief Initializer for GRIB Section 2 (Local Use Section).
///
/// This function allocates and configures GRIB Section 2 by:
/// - enabling the local definition section
/// - selecting the appropriate local definition number
/// - optionally injecting additional metadata for special template numbers
///
/// Two *virtual* template numbers are handled explicitly:
/// - `1001`: DestinE / ClimateDT local definition
/// - `1002`: DestinE / ExtremesDT local definition
///
/// These template numbers are **not part of the official ecCodes tables**.
/// They are used internally by the mars2grib encoder to emulate the
/// behavior of standard templates while injecting additional semantics
/// through Section 2.
///
/// All dictionary mutations are performed via `set_or_throw` to ensure
/// strict error checking.
///
/// @tparam SectionNumber   GRIB section number (expected to be 2)
/// @tparam TemplateNumber  GRIB template number or virtual template identifier
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
/// If any dictionary operation fails while preparing Section 2.
///
/// @note
/// Existing local definition content is not checked and may be overwritten.
///
template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void allocateTemplateNumber2(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    // Dictionary traits
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        // Enable local definition section (overwrite if already present)
        set_or_throw<long>(out, "setLocalDefinition", 1);

        // Select local definition number based on template number
        // Special handling for DestinE virtual templates
        if constexpr (TemplateNumber == 1001) {
            // Minimal Section 2
            set_or_throw<long>(out, "localDefinitionNumber", 1L);
            // DestinE metadata
            set_or_throw<long>(out, "productionStatusOfProcessedData", 12L);
            // ClimateDT dataset tag
            set_or_throw<std::string>(out, "dataset", "climate-dt");
        }
        else if constexpr (TemplateNumber == 1002) {
            // Minimal Section 2
            set_or_throw<long>(out, "localDefinitionNumber", 1L);
            // DestinE metadata
            set_or_throw<long>(out, "productionStatusOfProcessedData", 12L);
            // ExtremesDT dataset tag
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

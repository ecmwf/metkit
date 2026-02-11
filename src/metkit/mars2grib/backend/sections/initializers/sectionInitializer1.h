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
/// @file section1Initializer.h
/// @brief Placeholder initializer for GRIB Section 1.
///
/// This header defines a **section initializer** associated with
/// GRIB **Section 1** and **Template Number 1**.
///
/// Section 1 in GRIB messages contains identification and origin
/// metadata and, in the current mars2grib architecture, does not
/// require any concept-driven or dictionary-based initialization.
///
/// The initializer provided here is therefore a **no-op**, introduced
/// to preserve uniformity in the section-initializer registry and
/// dispatch infrastructure.
///
/// This file contains **no state**, **no encoding logic**, and performs
/// **no mutation** of the output dictionary.
///
/// @note
/// This initializer is expected to be removed or replaced once
/// Section 1 handling is either specialized or excluded from the
/// generic initialization pipeline.
///
/// @ingroup mars2grib_backend_sections
///
#pragma once

#include "metkit/mars2grib/backend/sections/initializers/sectionInitializerCore.h"

namespace metkit::mars2grib::backend::sections::initializers {

///
/// @brief No-op initializer for GRIB Section 1, Template 1.
///
/// This function conforms to the standard section initializer
/// signature but intentionally performs no operations.
///
/// It allows Section 1 to participate in the same compile-time
/// registry and dispatch mechanisms as other GRIB sections,
/// despite requiring no initialization logic.
///
/// @tparam SectionNumber   GRIB section number (expected to be 1)
/// @tparam TemplateNumber  GRIB template number (expected to be 1)
/// @tparam MarsDict_t      Type of the MARS dictionary
/// @tparam ParDict_t       Type of the parameter dictionary
/// @tparam OptDict_t       Type of the options dictionary
/// @tparam OutDict_t       Type of the output GRIB dictionary
///
/// @param mars Read-only MARS dictionary (unused)
/// @param par  Read-only parameter dictionary (unused)
/// @param opt  Read-only options dictionary (unused)
/// @param out  Output GRIB dictionary (unused)
///
/// @note
/// This function currently exists as a placeholder and may be
/// removed or specialized once Section 1 initialization is
/// handled explicitly.
///
template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void allocateTemplateNumber1(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    // No-op for Section 1 — placeholder initializer
}

}  // namespace metkit::mars2grib::backend::sections::initializers

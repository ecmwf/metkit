// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file section0Initializer.h
/// @brief Placeholder initializer for GRIB Section 0.
///
/// This header defines a **section initializer** associated with
/// GRIB **Section 0** and **Template Number 0**.
///
/// Section 0 in GRIB messages contains only metadata related to the
/// GRIB edition and message structure and does not require any
/// concept-driven encoding or dictionary-based initialization.
///
/// As such, the initializer provided here is intentionally a **no-op**.
/// It exists solely to satisfy the uniform section-initializer
/// infrastructure used by the mars2grib backend.
///
/// This file contains **no state**, **no logic**, and performs
/// **no mutation** of the output dictionary.
///
/// @note
/// This initializer is temporary and marked for removal once
/// Section 0 handling is fully excluded from the generic
/// section initialization pipeline.
///
/// @ingroup mars2grib_backend_sections
///
#pragma once

#include "metkit/mars2grib/backend/sections/initializers/sectionInitializerCore.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::sections::initializers {

///
/// @brief No-op initializer for GRIB Section 0, Template 0.
///
/// This function conforms to the standard section initializer
/// signature but intentionally performs no operations.
///
/// It is provided to allow Section 0 to participate in the same
/// compile-time registry and dispatch mechanisms as other GRIB
/// sections, even though no initialization is required.
///
/// @tparam SectionNumber   GRIB section number (expected to be 0)
/// @tparam TemplateNumber  GRIB template number (expected to be 0)
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
/// This function currently exists as a placeholder and is expected
/// to be removed.
///
template <std::size_t SectionNumber, std::size_t TemplateNumber, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void allocateTemplateNumber0(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    // No-op for Section 0 — placeholder initializer
}

}  // namespace metkit::mars2grib::backend::sections::initializers

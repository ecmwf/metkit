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
 * @file shapeOfTheEarth.h
 * @brief Deduction of the GRIB shape of the Earth.
 *
 * This header defines the deduction responsible for resolving the
 * GRIB `shapeOfTheEarth` key used to describe the reference system
 * of the Earth in GRIB messages.
 *
 * The deduction currently applies a fixed, deterministic value
 * corresponding to a spherical Earth with radius 6371229 m.
 *
 * Deductions:
 * - extract values from input dictionaries (currently unused)
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer values from MARS metadata
 * - apply configuration-based defaults
 * - validate against GRIB code tables
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: value defaulted by the deduction
 *
 * @section References
 * Concept:
 *   - @ref shapeOfTheEarthEncoding.h
 *
 * Related deductions:
 *   (none)
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// Tables
#include "metkit/mars2grib/backend/tables/shapeOfTheReferenceSystem.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `shapeOfTheEarth` key.
 *
 * @section Deduction contract
 * - Reads: none
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: none (deterministic)
 *
 * This deduction resolves the GRIB `shapeOfTheEarth` key by
 * applying a fixed, deterministic value corresponding to
 * a spherical Earth with radius 6371229 m.
 *
 * No inference from MARS, geometry, or options dictionaries
 * is currently performed.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam GeomDict_t Type of the geometry dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] geom Geometry dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return
 *   The resolved `ShapeOfTheReferenceSystem` enumeration value.
 *
 * @note
 * This function is the **single authoritative deduction**
 * for `shapeOfTheEarth`.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: extensibility][prio: low]
 * - Introduce inference or configuration-based selection when
 *   non-spherical Earth representations are required.
 */
template <class MarsDict_t, class ParDict_t, class GeomDict_t, class OptDict_t>
tables::ShapeOfTheReferenceSystem resolve_ShapeOfTheEarth_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                   const GeomDict_t& geom, const OptDict_t& opt) {
    // Default value
    tables::ShapeOfTheReferenceSystem shapeOfTheEarth = tables::ShapeOfTheReferenceSystem::EarthSphericalRadius6371229;

    // Logging of the channel
    MARS2GRIB_LOG_RESOLVE([&]() {
        std::string logMsg = "`shapeOfTheEarth` defaulted from input dictionaries: value='";
        logMsg += tables::enum2name_ShapeOfTheReferenceSystem_or_throw(shapeOfTheEarth);
        logMsg += "'";
        return logMsg;
    }());

    // Success exit point
    return shapeOfTheEarth;
};

}  // namespace metkit::mars2grib::backend::deductions

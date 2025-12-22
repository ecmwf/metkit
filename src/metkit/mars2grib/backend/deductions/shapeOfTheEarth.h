/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

// System includes
#include <string>

// External libraries
#include "eckit/log/Log.h"

// Tables
#include "metkit/mars2grib/backend/tables/shapeOfTheReferenceSystem.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB Local Tables Version Number.
 *
 * This deduction determines the value of the GRIB
 * `localTablesVersionNumber` key.
 *
 * At present, the deduction is **hard-coded** to return `0`, indicating
 * that no local tables are used and that only standard GRIB tables apply.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `localTablesVersionNumber` in the encoder.
 * Any future logic related to local GRIB tables **must be implemented here**.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The local tables version number.
 *         Currently always returns `0`.
 *
 * @note
 * - A value of `0` indicates that no local GRIB tables are in use.
 * - This behavior is fully deterministic and does not depend on input data.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: extensibility][prio: low]
 * - Introduce proper deduction logic when local GRIB tables are required.
 * - Validate consistency between local tables version and centre/subCentre.
 * - Consider deriving this value from configuration or encoder options
 *   rather than hard-coding it.
 */
template <class MarsDict_t, class ParDict_t, class GeomDict_t, class OptDict_t>
tables::ShapeOfTheReferenceSystem resolve_ShapeOfTheEarth_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                   const GeomDict_t& geom, const OptDict_t& opt) {

    tables::ShapeOfTheReferenceSystem shapeOfTheEarth = tables::ShapeOfTheReferenceSystem::EarthSphericalRadius6371229;

    // Logging of the channel
    MARS2GRIB_LOG_RESOLVE([&]() {
        std::string logMsg = "shapeOfTheEarth: defaulted to: ";
        logMsg += tables::enum2name_ShapeOfTheReferenceSystem_or_throw(shapeOfTheEarth);
        return logMsg;
    }());

    return shapeOfTheEarth;
};

}  // namespace metkit::mars2grib::backend::deductions

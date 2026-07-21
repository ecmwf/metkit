/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file common.h
/// @brief Shared deduction-local temporal DTOs.
///
/// This header contains only the small value types shared by multiple temporal
/// deductions. It intentionally does NOT depend on ProductTimeSpec model types,
/// so deductions remain buildable without the backend model layer.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Deduction-local time duration.
///
/// This type represents one non-negative time length paired with the GRIB time
/// unit used by the deduction layer. It is shared by `step`, `timespan`,
/// `stattype`, and `timeIncrement` deductions.
///
struct TimeDuration {
    long length{0};
    tables::TimeUnit unit{tables::TimeUnit::Second};
};

}  // namespace metkit::mars2grib::backend::deductions

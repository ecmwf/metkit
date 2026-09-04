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
/// @file timeDurationHelpers.h
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

#include "metkit/mars2grib/backend/deductions/common.h"


namespace metkit::mars2grib::backend::deductions::detail {


///
/// @brief Canonicalize a non-negative elapsed duration expressed in seconds.
///
/// Positive whole-hour values are represented in hours. Zero and non-hour-
/// aligned values are represented in seconds.
///
/// @param[in] seconds Elapsed duration in seconds.
/// @param[in] key     Human-readable source-key name used in diagnostics.
/// @return Canonical elapsed duration.
/// @throws Mars2GribDeductionException if `seconds` is negative.
///
inline TimeDuration canonicalElapsedDuration(long seconds, const std::string& key) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (seconds < 0) {
        throw Mars2GribDeductionException("`" + key + "` must be non-negative", Here());
    }
    if (seconds > 0 && seconds % 3600L == 0) {
        return TimeDuration{seconds / 3600L, tables::TimeUnit::Hour};
    }
    return TimeDuration{seconds, tables::TimeUnit::Second};
}

}  // namespace metkit::mars2grib::backend::deductions::detail

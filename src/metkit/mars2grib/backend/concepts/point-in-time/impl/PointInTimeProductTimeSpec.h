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
/// @file impl/PointInTimeProductTimeSpec.h
/// @brief Point-in-time-specific transport struct built from `ProductTimeSpec`.
///
/// Exposes `PointInTimeProductTimeSpec` plus the pure builder
/// `build_PointInTimeProductTimeSpec_or_throw`, which turns one final immutable
/// `backend::models::product_time_spec::ProductTimeSpec` into the point-in-
/// time-facing transport struct consumed by the point-in-time concept.
///
/// This header owns only the point-in-time-facing transport type and the
/// builder. It does NOT perform GRIB encoding itself.
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

#include <exception>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_::impl {

///
/// @brief Point-in-time-facing transport struct derived from `ProductTimeSpec`.
///
/// The struct contains only the runtime temporal metadata needed by the
/// point-in-time concept.
///
/// `forecastTime` is currently materialized only in hours.
///
struct PointInTimeProductTimeSpec {
    /// @brief Forecast-time offset expressed in whole hours.
    deductions::TimeDuration forecastTime{0, tables::TimeUnit::Hour};
};

///
/// @brief Build the point-in-time transport struct from one final
///        `ProductTimeSpec`.
///
/// The builder reads only the final immutable backend-model `ProductTimeSpec`
/// and materializes the point-in-time-facing representation consumed by the
/// point-in-time concept.
///
/// Build rules:
/// - only `Instant` ProductTimeSpec values are accepted;
/// - `forecastTime` is the distance between
///   `anchor.referenceDateTime` and `domain.domainEndDateTime`;
/// - `forecastTime` must be non-negative and a whole number of hours.
///
/// @param[in] spec Final immutable backend-model `ProductTimeSpec`.
/// @return Fully populated `PointInTimeProductTimeSpec`.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribGenericException on
///         non-instant input, invalid forecast-time arithmetic, or any
///         unexpected failure, with the original cause preserved through nested
///         exceptions.
///
inline PointInTimeProductTimeSpec build_PointInTimeProductTimeSpec_or_throw(
    const models::product_time_spec::ProductTimeSpec& spec) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        if (spec.shapeType() != ProductTimeSpecShapeKind::Instant) {
            std::ostringstream oss;
            oss << "Point-in-time backend requires an Instant `ProductTimeSpec`. Timespec is: " << spec.to_json();
            throw Mars2GribGenericException(oss.str(), Here());
        }

        const long forecastTimeInSeconds = static_cast<long>(
            static_cast<eckit::Second>(spec.domain().domainEndDateTime - spec.anchor().referenceDateTime));

        if (forecastTimeInSeconds < 0) {
            throw Mars2GribGenericException("`PointInTimeProductTimeSpec` forecastTime must be non-negative", Here());
        }

        if (forecastTimeInSeconds % 3600 != 0) {
            throw Mars2GribGenericException("`PointInTimeProductTimeSpec` forecastTime must be a whole number of hours",
                                            Here());
        }

        PointInTimeProductTimeSpec out;
        out.forecastTime = deductions::TimeDuration{forecastTimeInSeconds / 3600, tables::TimeUnit::Hour};
        return out;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Failed to build `PointInTimeProductTimeSpec` from `ProductTimeSpec`", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_::impl

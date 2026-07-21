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
/// @file impl/StatisticsProductTimeSpec.h
/// @brief Statistics-specific transport struct built from `ProductTimeSpec`.
///
/// Exposes `StatisticsProductTimeSpec` plus the pure builder
/// `build_StatisticsProductTimeSpec_or_throw`, which turns one final immutable
/// `backend::models::product_time_spec::ProductTimeSpec` into the statistics-
/// facing transport struct consumed by the statistics concept.
///
/// This header owns only the statistics-facing transport type and the builder.
/// It does NOT perform GRIB encoding itself.
///
/// The resulting representation preserves the canonical ProductTimeSpec window
/// ordering (outermost -> innermost).
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

#include <exception>
#include <vector>

#include "eckit/types/DateTime.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_::impl {

///
/// @brief Statistics-facing transport struct derived from `ProductTimeSpec`.
///
/// The struct combines:
/// - runtime temporal metadata used by the statistics concept;
/// - the GRIB statistical-processing arrays written during preset/allocate
///   stages.
///
/// All vector members have length `numberOfTimeRanges` and preserve the
/// canonical `ProductTimeSpec` window ordering (outermost -> innermost).
///
/// `forecastTime` is currently materialized only in hours.
///
struct StatisticsProductTimeSpec {
    /// @brief Forecast-time offset expressed in whole hours.
    deductions::TimeDuration forecastTime{0, tables::TimeUnit::Hour};

    /// @brief End datetime of the overall statistical interval.
    eckit::DateTime endOfOverallTimeInterval{};

    /// @brief Number of encoded time ranges.
    long numberOfTimeRanges{0};

    /// @brief GRIB `typeOfStatisticalProcessing` vector.
    std::vector<long> typeOfStatisticalProcessing;

    /// @brief GRIB `typeOfTimeIncrement` vector.
    std::vector<long> typeOfTimeIncrement;

    /// @brief GRIB `indicatorOfUnitForTimeRange` vector.
    std::vector<long> indicatorOfUnitForTimeRange;

    /// @brief GRIB `lengthOfTimeRange` vector.
    std::vector<long> lengthOfTimeRange;

    /// @brief GRIB `indicatorOfUnitForTimeIncrement` vector.
    std::vector<long> indicatorOfUnitForTimeIncrement;

    /// @brief GRIB `timeIncrement` vector.
    std::vector<long> lengthOfTimeIncrement;
};

///
/// @brief Build the statistics transport struct from one final `ProductTimeSpec`.
///
/// The builder reads only the final immutable backend-model `ProductTimeSpec`
/// and materializes the statistics-facing representation consumed by the
/// statistics concept.
///
/// Build rules:
/// - `Instant` ProductTimeSpec values are rejected;
/// - `forecastTime` is the distance between
///   `anchor.referenceDateTime` and `domain.domainStartDateTime`;
/// - `forecastTime` must be non-negative and a whole number of hours;
/// - `endOfOverallTimeInterval` is `domain.domainEndDateTime`;
/// - the array members are copied directly from the canonical windows in
///   outermost -> innermost order.
///
/// @param[in] spec Final immutable backend-model `ProductTimeSpec`.
/// @return Fully populated `StatisticsProductTimeSpec`.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribGenericException on
///         instant input, invalid forecast-time arithmetic, or any unexpected
///         failure, with the original cause preserved through nested
///         exceptions.
///
inline StatisticsProductTimeSpec build_StatisticsProductTimeSpec_or_throw(
    const models::product_time_spec::ProductTimeSpec& spec) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        if (spec.shapeType() == ProductTimeSpecShapeKind::InstantTimespanMissing ||
            spec.shapeType() == ProductTimeSpecShapeKind::InstantTimespanNone) {
            throw Mars2GribGenericException(
                "Statistics backend cannot build a `StatisticsProductTimeSpec` from an Instant `ProductTimeSpec`",
                Here());
        }

        const long forecastTimeInSeconds = static_cast<long>(
            static_cast<eckit::Second>(spec.domain().domainStartDateTime - spec.anchor().referenceDateTime));

        if (forecastTimeInSeconds < 0) {
            throw Mars2GribGenericException("`StatisticsProductTimeSpec` forecastTime must be non-negative", Here());
        }

        if (forecastTimeInSeconds % 3600 != 0) {
            throw Mars2GribGenericException("`StatisticsProductTimeSpec` forecastTime must be a whole number of hours",
                                            Here());
        }

        StatisticsProductTimeSpec out;
        out.forecastTime             = deductions::TimeDuration{forecastTimeInSeconds / 3600, tables::TimeUnit::Hour};
        out.endOfOverallTimeInterval = spec.domain().domainEndDateTime;

        const auto& windows    = spec.windows().values;
        out.numberOfTimeRanges = static_cast<long>(windows.size());

        out.typeOfStatisticalProcessing.reserve(windows.size());
        out.typeOfTimeIncrement.reserve(windows.size());
        out.indicatorOfUnitForTimeRange.reserve(windows.size());
        out.lengthOfTimeRange.reserve(windows.size());
        out.indicatorOfUnitForTimeIncrement.reserve(windows.size());
        out.lengthOfTimeIncrement.reserve(windows.size());

        for (const auto& window : windows) {
            out.typeOfStatisticalProcessing.push_back(static_cast<long>(window.typeOfStatisticalProcessing));
            out.typeOfTimeIncrement.push_back(static_cast<long>(window.typeOfTimeIncrement));
            out.indicatorOfUnitForTimeRange.push_back(static_cast<long>(window.timeRange.unit));
            out.lengthOfTimeRange.push_back(window.timeRange.length);
            out.indicatorOfUnitForTimeIncrement.push_back(static_cast<long>(window.timeIncrement.unit));
            out.lengthOfTimeIncrement.push_back(window.timeIncrement.length);
        }

        return out;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Failed to build `StatisticsProductTimeSpec` from `ProductTimeSpec`", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_::impl

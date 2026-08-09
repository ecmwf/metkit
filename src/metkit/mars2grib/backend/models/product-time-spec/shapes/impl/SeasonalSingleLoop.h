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
/// @file SeasonalSingleLoop.h
/// @brief Matcher and leaf builder for a seasonal single-loop statistic.
///
/// This header is the authoritative implementation of the
/// `SeasonalSingleLoop` ProductTimeSpec shape. It deliberately owns both the
/// matcher and the leaf builder for this case.
///
/// The matcher exposes each structural and regime condition through a
/// semantically named Boolean. The builder keeps the full window-construction
/// flow local: range selection, shape-specific validation, increment
/// resolution, window creation, and ordering are visible here.
///
/// Only genuinely cross-cutting semantics such as `typeOfTimeIncrement`,
/// default increment deduction, and temporal arithmetic are delegated. All
/// failures are nested in `Mars2GribModelException` with the normalized input
/// snapshot.
///
/// @ingroup mars2grib_product_time_spec_shapes
///

#pragma once

#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ForecastLeadUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/TimeIncrement.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeUtils.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Match a seasonal single-loop statistical representation.
 *
 * The shape matches when:
 *
 * - the resolved domain classification is `SeasonalForecastDomain`;
 * - the normalized input is seasonal;
 * - the product is not synoptic;
 * - MARS semantics classify the product as forecast;
 * - `timespan` is explicitly `none`, or it is missing and accepted by the
 *   shape policy;
 * - no outer `stattype` blocks are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_SeasonalSingleLoop_Shape(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isSeasonal                = product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic             = !input.isSynoptic;
        const bool isForecast                = input.simulationType == SimulationType::Forecast;
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) ||
            timespanIsMissingAndAllowed(input, input.allowMissingTimespanForStatisticalProduct);
        const bool hasNoStattypeBlocks = input.stattype.empty();

        return isSeasonal && isNotSynoptic && isForecast &&
               hasAcceptedTimespanRepresentation && !hasNoStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalSingleLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Build one canonical seasonal statistical window.
 *
 * The builder keeps the complete high-level flow visible:
 *
 * 1. use the intrinsic one-calendar-month seasonal window range;
 * 2. resolve explicit, missing, or defaulted increment semantics;
 * 3. construct the canonical window directly;
 * 4. return the one-element window vector.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed absolute seasonal ProductTimeSpec domain.
 * @return One canonical seasonal statistical window.
 * @throws Mars2GribModelException If range or increment resolution fails.
 */
inline ProductTimeSpecOuterTimeRange build_SeasonalSingleLoop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)classification;
        const TimeDuration timeRange = oneMonth();

        return ProductTimeSpecOuterTimeRange{ProductTimeSpecOuterTimeRangeAvailability::Available, timeRange};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_SeasonalSingleLoop_ShapeOuterTimeRange`", input.to_json(),
                                    Here()));
    }
}

inline ProductTimeSpecShape build_SeasonalSingleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveIfsInnerIncrement;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)classification;
        (void)anchor;
        (void)outerTimeRange;
        (void)domain;

        const auto timeRange = oneMonth();
        const auto resolvedIncrement = resolveIfsInnerIncrement(input, timeRange, false);

        ProductTimeSpecWindow window{input.innerMostTypeOfStatisticalProcessing, resolvedIncrement.typeOfTimeIncrement,
                                     timeRange, resolvedIncrement.timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_SeasonalSingleLoop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

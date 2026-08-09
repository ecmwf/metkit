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
/// @file SeasonalMultiloop.h
/// @brief Matcher and leaf builder for a seasonal multi-loop statistic.
///
/// This header is the authoritative implementation of the
/// `SeasonalMultiloop` ProductTimeSpec shape. It deliberately owns both the
/// matcher and the leaf builder for this case.
///
/// The matcher exposes each structural and regime condition through a
/// semantically named Boolean. The builder keeps the full window-construction
/// flow local: range selection, shape-specific validation, increment
/// resolution, window creation, and ordering are visible here.
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
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Match a seasonal multi-loop statistical representation.
 *
 * The shape matches when:
 *
 * - the resolved domain classification is `SeasonalForecastDomain`;
 * - the normalized input is seasonal;
 * - the product is not synoptic;
 * - MARS semantics classify the product as forecast;
 * - `timespan` contains the inner-loop duration;
 * - one or more outer `stattype` blocks are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_SeasonalMultiloop_Shape(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isSeasonal                = product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic             = !input.isSynoptic;
        const bool isForecast                = input.simulationType == SimulationType::Forecast;
        const bool hasDurationTimespan       = input.timespan.kind == TimespanKind::Duration;
        const bool hasOuterStattypeBlocks    = !input.stattype.empty();

        return isSeasonal && isNotSynoptic && isForecast && hasDurationTimespan &&
               hasOuterStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalMultiloop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Build all canonical seasonal windows in outermost-to-innermost order.
 *
 * The high-level multi-loop algorithm is intentionally implemented here:
 *
 * 1. create one canonical outer window for every `stattype` block;
 * 2. resolve the innermost range from `timespan`;
 * 3. resolve the innermost explicit, missing, or defaulted increment;
 * 4. append the innermost canonical window;
 * 5. assign each outer window increment from the next inner window range.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed absolute seasonal ProductTimeSpec domain.
 * @return Canonical windows ordered outermost to innermost.
 * @throws Mars2GribModelException If any range or increment is invalid.
 */
inline ProductTimeSpecOuterTimeRange build_SeasonalMultiloop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;
        if (input.stattype.empty()) {
            throw Mars2GribModelException("Cannot build SeasonalMultiloop shape with no outer stattype blocks",
                                          input.to_json(), Here());
        }

        if (input.stattype.size() > 2) {
            throw Mars2GribModelException("Cannot build SeasonalMultiloop shape with more than 2 stattype blocks",
                                          input.to_json(), Here());
        }

        const TimeDuration outerTimeRange = input.stattype.front().timeRange;

        return ProductTimeSpecOuterTimeRange{ProductTimeSpecOuterTimeRangeAvailability::Available, outerTimeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_SeasonalMultiloop_ShapeOuterTimeRange`",
                                                       input.to_json(), Here()));
    }
}

inline ProductTimeSpecShape build_SeasonalMultiloop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveIfsInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::timespanDuration;

    try {
        (void)classification;
        (void)anchor;
        (void)outerTimeRange;
        (void)domain;

        std::vector<ProductTimeSpecWindow> windows;
        windows.reserve(input.stattype.size() + 1);

        for (const auto& stattypeBlock : input.stattype) {
            const auto& timeRange = stattypeBlock.timeRange;
            windows.push_back(ProductTimeSpecWindow{stattypeBlock.typeOfStatisticalProcessing,
                                                    typeOfTimeIncrementForWindow(input, true, false, timeRange), timeRange,
                                                    missingIncrement()});
        }

        const auto innermostTimeRange = timespanDuration(input);
        const auto resolvedInnermostIncrement = resolveIfsInnerIncrement(input, innermostTimeRange, true);

        windows.push_back(ProductTimeSpecWindow{input.innerMostTypeOfStatisticalProcessing,
                                                resolvedInnermostIncrement.typeOfTimeIncrement, innermostTimeRange,
                                                resolvedInnermostIncrement.timeIncrement});

        for (std::size_t windowIndex = 0; windowIndex + 1 < windows.size(); ++windowIndex) {
            windows[windowIndex].timeIncrement = windows[windowIndex + 1].timeRange;
        }

        return ProductTimeSpecShape{windows};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_SeasonalMultiloop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

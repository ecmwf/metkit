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
/// @file IFSStandardMultiLoop.h
/// @brief Matcher and leaf builder for a standard IFS multi-loop statistic.
///
/// This header is the authoritative implementation of the `IFSStandardMultiLoop` ProductTimeSpec shape. It deliberately
/// owns both the matcher and the leaf builder for this case.
///
/// The matcher exposes each structural and regime condition through a semantically named Boolean. The builder keeps the
/// full window-construction flow local: range selection, shape-specific validation, increment resolution, window
/// creation, and ordering are visible here.
///
/// Only genuinely cross-cutting semantics—such as `typeOfTimeIncrement`, default increment deduction, and temporal
/// arithmetic—are delegated. All failures are nested in `Mars2GribModelException` with the normalized input snapshot.
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
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Match a standard IFS multi-loop statistical representation.
 *
 * The shape matches when:
 *
 * - the regime is IFS;
 * - the resolved domain classification is `ForecastDomain`;
 * - the normalized input is not seasonal;
 * - the product is not synoptic;
 * - `timespan` contains the innermost loop duration;
 * - one or more outer `stattype` blocks are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_IFSStandardMultiLoop_Shape(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        const bool isIfs                  = input.regime == SimulationRegime::IFS;
        const bool isForecast             = input.simulationType == SimulationType::Forecast;
        const bool isNotSeasonal          = !product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic          = !input.isSynoptic;
        const bool hasDurationTimespan    = input.timespan.kind == TimespanKind::Duration;
        const bool hasOuterStattypeBlocks = !input.stattype.empty();

        return isIfs && isForecast && isNotSeasonal && isNotSynoptic && hasDurationTimespan &&
               hasOuterStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_IFSStandardMultiLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Build all canonical IFS windows in outermost-to-innermost order.
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
 * @param[in] domain Already constructed absolute ProductTimeSpec domain.
 * @return Canonical windows ordered outermost to innermost.
 * @throws Mars2GribModelException If any range or increment is invalid.
 */
inline ProductTimeSpecShapeStage1 build_IFSStandardMultiLoop_ShapeStage1(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveIfsInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::timespanDuration;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        if (input.stattype.empty()) {
            throw Mars2GribModelException("Cannot build IFSStandardMultiLoop shape with no outer stattype blocks",
                                          input.to_json(), Here());
        }

        if (input.stattype.size() > 2) {
            throw Mars2GribModelException("Cannot build IFSStandardMultiLoop shape with more than 2 stattype blocks",
                                          input.to_json(), Here());
        }

        std::vector<ProductTimeSpecWindow> windows;
        windows.reserve(input.stattype.size() + 1);

        for (const auto& stattypeBlock : input.stattype) {
            ProductTimeSpecWindow outerWindow{stattypeBlock.typeOfStatisticalProcessing,
                                              typeOfTimeIncrementForWindow(input, true, false, stattypeBlock.timeRange),
                                              stattypeBlock.timeRange, missingIncrement()};

            windows.push_back(outerWindow);
        }

        const TimeDuration innermostTimeRange = timespanDuration(input);

        const ResolvedInnerIncrement resolvedInnermostIncrement = resolveIfsInnerIncrement(input, innermostTimeRange, true);

        ProductTimeSpecWindow innermostWindow{input.innerMostTypeOfStatisticalProcessing,
                                              resolvedInnermostIncrement.typeOfTimeIncrement, innermostTimeRange,
                                              resolvedInnermostIncrement.timeIncrement};

        windows.push_back(innermostWindow);

        for (std::size_t windowIndex = 0; windowIndex + 1 < windows.size(); ++windowIndex) {
            windows[windowIndex].timeIncrement = windows[windowIndex + 1].timeRange;
        }

        return ProductTimeSpecShapeStage1{windows};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_IFSStandardMultiLoop_ShapeStage1`",
                                                       input.to_json(), Here()));
    }
}

inline ProductTimeSpecShape build_IFSStandardMultiLoop_ShapeFinal(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecShapeStage1& shapeStage1,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;
        return ProductTimeSpecShape{shapeStage1.values};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_IFSStandardMultiLoop_ShapeFinal`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

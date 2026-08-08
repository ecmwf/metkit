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
/// @file IFSFakeSingleLoopDoubleLoop.h
/// @brief Matcher and leaf builder for an apparent IFS single loop requiring two canonical loops.
///
/// This header is the authoritative implementation of the `IFSFakeSingleLoopDoubleLoop` ProductTimeSpec shape. It
/// deliberately owns both the matcher and the leaf builder for this case.
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
 * @brief Match an apparent IFS single loop that requires two canonical windows.
 *
 * The shape matches when:
 *
 * - the regime is IFS;
 * - the resolved domain classification is `ForecastDomain`;
 * - the normalized input is not seasonal;
 * - the product is not synoptic;
 * - `timespan` contains an explicit duration;
 * - no source `stattype` blocks are present;
 * - the fake-double-loop source representation is not required;
 * - the `(class, stream, type, paramId)` rule requires a fake second loop.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_IFSFakeSingleLoopDoubleLoop_Shape(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        const bool isIfs                        = input.regime == SimulationRegime::IFS;
        const bool isForecast                   = input.simulationType == SimulationType::Forecast;
        const bool isNotSeasonal                = !product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic                = !input.isSynoptic;
        const bool hasDurationTimespan          = input.timespan.kind == TimespanKind::Duration;
        const bool hasNoStattypeBlocks          = input.stattype.empty();
        const bool requiresFakeDoubleLoop       = input.requiresFakeDoubleLoopSingleLoopRepresentation;
        const bool doesNotRequireFakeDoubleLoop = !requiresFakeDoubleLoop;
        const bool requiresFakeSecondLoop       = input.requiresFakeSingleLoopDoubleLoopRepresentation;

        return isIfs && isForecast && isNotSeasonal && isNotSynoptic && hasDurationTimespan &&
               hasNoStattypeBlocks && doesNotRequireFakeDoubleLoop && requiresFakeSecondLoop;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `match_IFSFakeSingleLoopDoubleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Build two canonical IFS windows over the same source timespan.
 *
 * The complete fake-single-loop transformation is explicit:
 *
 * 1. obtain the shared time range from `timespan`;
 * 2. create the synthetic outer window;
 * 3. resolve the real innermost increment;
 * 4. create the real innermost window;
 * 5. assign the outer increment from the innermost range;
 * 6. return both windows in outermost-to-innermost order.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed absolute ProductTimeSpec domain.
 * @return Exactly two canonical windows.
 * @throws Mars2GribModelException If range or increment resolution fails.
 */
inline ProductTimeSpecShapeStage1 build_IFSFakeSingleLoopDoubleLoop_ShapeStage1(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveIfsInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::timespanDuration;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;
        const TimeDuration sharedTimeRange = timespanDuration(input);

        ProductTimeSpecWindow outerWindow{input.innerMostTypeOfStatisticalProcessing,
                                          typeOfTimeIncrementForWindow(input, true, false, sharedTimeRange),
                                          sharedTimeRange, sharedTimeRange};

        const ResolvedInnerIncrement resolvedInnermostIncrement = resolveIfsInnerIncrement(input, sharedTimeRange, true);

        ProductTimeSpecWindow innermostWindow{input.innerMostTypeOfStatisticalProcessing,
                                              resolvedInnermostIncrement.typeOfTimeIncrement, sharedTimeRange,
                                              resolvedInnermostIncrement.timeIncrement};

        outerWindow.timeIncrement = innermostWindow.timeRange;

        return ProductTimeSpecShapeStage1{{outerWindow, innermostWindow}};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_IFSFakeSingleLoopDoubleLoop_ShapeStage1`",
                                    input.to_json(), Here()));
    }
}

inline ProductTimeSpecShape build_IFSFakeSingleLoopDoubleLoop_ShapeFinal(
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
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_IFSFakeSingleLoopDoubleLoop_ShapeFinal`",
                                    input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

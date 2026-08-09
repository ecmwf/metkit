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
/// @file IFSFromStartSingleLoopAtZero.h
/// @brief Matcher and leaf builder for an IFS from-start single-loop statistic.
///
/// This header is the authoritative implementation of the `IFSFromStartSingleLoop` ProductTimeSpec shape. It
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
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeUtils.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Match an IFS single-loop statistic using from-start semantics.
 *
 * The shape matches when:
 *
 * - the regime is IFS;
 * - the resolved domain classification is `ForecastDomain`;
 * - the normalized input is not seasonal;
 * - the product is not synoptic;
 * - `timespan` uses from-start semantics;
 * - no outer `stattype` blocks are present;
 * - step is zero.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_IFSFromStartSingleLoopAtZero_Shape(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        const bool isIfs                 = input.regime == SimulationRegime::IFS;
        const bool isForecast            = input.simulationType == SimulationType::Forecast;
        const bool isNotSeasonal         = !product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic         = !input.isSynoptic;
        const bool usesFromStartTimespan = input.timespan.kind == TimespanKind::FromStart;
        const bool hasNoStattypeBlocks   = input.stattype.empty();
        const bool hasZeroStep           = product_time_spec::detail::stepIsZero(input);

        const bool stepZeroIsAllowed  = input.allowZeroLengthFsWindow;
        const bool operationIsAllowed = input.isAllowedInnerTypeOfStatisticalProcessingAtStepZero;


        return isIfs && isForecast && isNotSeasonal && isNotSynoptic && usesFromStartTimespan &&
               hasNoStattypeBlocks && hasZeroStep && stepZeroIsAllowed && operationIsAllowed;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `match_IFSFromStartSingleLoopAtZero_Shape`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Build one IFS from-start window from the resolved absolute domain.
 *
 * The builder performs the full shape-specific flow:
 *
 * 1. calculate the elapsed range between domain start and domain end;
 * 2. identify the accepted zero-length special case;
 * 3. resolve explicit, missing, or defaulted increment semantics;
 * 4. construct and return the one canonical window.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed absolute ProductTimeSpec domain.
 * @return One canonical from-start window.
 * @throws Mars2GribModelException If domain arithmetic or increment resolution fails.
 */
inline ProductTimeSpecOuterTimeRange build_IFSFromStartSingleLoopAtZero_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)input;
        (void)classification;
        return ProductTimeSpecOuterTimeRange{ProductTimeSpecOuterTimeRangeAvailability::Deferred, std::nullopt};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_IFSFromStartSingleLoopAtZero_ShapeOuterTimeRange`",
                                                       input.to_json(), Here()));
    }
}

inline ProductTimeSpecShape build_IFSFromStartSingleLoopAtZero_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveIfsInnerIncrement;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::durationBetween;

    try {
        (void)classification;
        (void)anchor;
        (void)outerTimeRange;
        const TimeDuration timeRange = durationBetween(domain.domainStartDateTime, domain.domainEndDateTime);

        const bool isZeroLengthFromStart = timeRange.length == 0;

        const ResolvedInnerIncrement resolvedIncrement = resolveIfsInnerIncrement(input, timeRange, false,
                                                                                  isZeroLengthFromStart);

        ProductTimeSpecWindow window{input.innerMostTypeOfStatisticalProcessing, resolvedIncrement.typeOfTimeIncrement,
                                     timeRange, resolvedIncrement.timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_IFSFromStartSingleLoopAtZero_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

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
/// @file IFSSynopticSingleLoop.h
/// @brief Matcher and leaf builder for an IFS synoptic-analysis single-loop statistic.
///
/// This header is the authoritative implementation of the `IFSSynopticSingleLoop` ProductTimeSpec shape. It
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
 * @brief Match an IFS synoptic-analysis single-loop statistic.
 *
 * The shape matches when:
 *
 * - the regime is IFS;
 * - the normalized input is not seasonal;
 * - the product is synoptic;
 * - the product is an analysis;
 * - the resolved domain classification is `SynopticAnalysisDomain`;
 * - no outer `stattype` blocks are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_IFSSynopticSingleLoop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isIfs               = input.regime == SimulationRegime::IFS;
        const bool isNotSeasonal       = !product_time_spec::detail::isSeasonal(input);
        const bool isSynoptic          = input.isSynoptic;
        const bool isAnalysis          = input.simulationType == SimulationType::Analysis;
        const bool hasNoStattypeBlocks = input.stattype.empty();

        return isIfs && isNotSeasonal && isSynoptic && isAnalysis && hasNoStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_IFSSynopticSingleLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Build the intrinsic one-month synoptic-analysis window.
 *
 * The builder keeps all shape-level decisions visible:
 *
 * 1. resolve and validate the intrinsic twenty-four-hour increment;
 * 2. assign the canonical one-calendar-month range;
 * 3. construct the single canonical statistical window.
 *
 * The absolute synoptic interval has already been constructed by the domain
 * builder and is intentionally not recomputed here.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed calendar-aligned synoptic domain.
 * @return One canonical synoptic-analysis window.
 * @throws Mars2GribModelException If redundant increment validation fails.
 */
inline ProductTimeSpecOuterTimeRange build_IFSSynopticSingleLoop_ShapeOuterTimeRange(
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
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_IFSSynopticSingleLoop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

inline ProductTimeSpecShape build_IFSSynopticSingleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveSynopticIncrement;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)classification;
        (void)anchor;
        (void)outerTimeRange;
        (void)domain;

        const auto resolvedIncrement = resolveSynopticIncrement(input);
        const auto timeRange         = oneMonth();

        ProductTimeSpecWindow window{input.innerMostTypeOfStatisticalProcessing, resolvedIncrement.typeOfTimeIncrement,
                                     timeRange, resolvedIncrement.timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_IFSSynopticSingleLoop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

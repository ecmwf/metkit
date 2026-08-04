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
/// @file IFSStandardSingleLoop.h
/// @brief Matcher and leaf builder for a standard IFS single-loop statistic.
///
/// This header is the authoritative implementation of the `IFSStandardSingleLoop` ProductTimeSpec shape. It
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
 * @brief Match a standard IFS single-loop statistical representation.
 *
 * The shape matches when:
 *
 * - the regime is IFS;
 * - the resolved domain classification is `ForecastDomain`;
 * - the normalized input is not seasonal;
 * - the product is not synoptic;
 * - `timespan` contains an explicit duration;
 * - no outer `stattype` blocks are present;
 * - the product does not require the fake-double-loop source representation;
 * - the product-specific whitelist does not require a fake second canonical loop.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_IFSStandardSingleLoop_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind& domainKind) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        const bool isIfs                                  = input.regime == SimulationRegime::IFS;
        const bool hasForecastDomain                      = domainKind == ProductTimeSpecDomainKind::ForecastDomain;
        const bool isNotSeasonal                          = !product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic                          = !input.isSynoptic;
        const bool hasDurationTimespan                    = input.timespan.kind == TimespanKind::Duration;
        const bool hasNoStattypeBlocks                    = input.stattype.empty();
        const bool requiresFakeDoubleLoop                 = input.requiresFakeDoubleLoopSingleLoopRepresentation;
        const bool requiresFakeSecondLoop                 = input.requiresFakeSingleLoopDoubleLoopRepresentation;
        const bool doesNotRequireFakeDoubleLoop           = !requiresFakeDoubleLoop;
        const bool doesNotRequireFakeSingleLoopDoubleLoop = !requiresFakeSecondLoop;

        return isIfs && hasForecastDomain && isNotSeasonal && isNotSynoptic && hasDurationTimespan &&
               hasNoStattypeBlocks && doesNotRequireFakeDoubleLoop && doesNotRequireFakeSingleLoopDoubleLoop;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_IFSStandardSingleLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Build one canonical IFS statistical window.
 *
 * The builder keeps the complete high-level flow visible:
 *
 * 1. obtain the innermost range from `timespan`;
 * 2. resolve explicit, missing, or defaulted increment semantics;
 * 3. construct the canonical window directly;
 * 4. return the one-element window vector.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed absolute ProductTimeSpec domain.
 * @return One canonical IFS statistical window.
 * @throws Mars2GribModelException If range or increment resolution fails.
 */
inline std::vector<ProductTimeSpecWindow> build_IFSStandardSingleLoop_Shape(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::resolveIfsInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::timespanDuration;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const TimeDuration timeRange = timespanDuration(input);

        const ResolvedInnerIncrement resolvedIncrement = resolveIfsInnerIncrement(input, domain, timeRange, false);

        ProductTimeSpecWindow window{input.innerMostTypeOfStatisticalProcessing, resolvedIncrement.typeOfTimeIncrement,
                                     timeRange, resolvedIncrement.timeIncrement};

        return {window};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_IFSStandardSingleLoop_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

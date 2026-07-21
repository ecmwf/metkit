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
/// @file AIFSFakeDoubleLoopSingleLoop.h
/// @brief Matcher and leaf builder for the AIFS fake-double-loop single-loop shape.
///
/// This header is the authoritative implementation of the `AIFSFakeDoubleLoopSingleLoop` ProductTimeSpec shape. It
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
#include "metkit/mars2grib/backend/models/product-time-spec/detail/TimeIncrement.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainUtils.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Match the AIFS fake-double-loop single-loop shape.
 *
 * The shape matches when:
 *
 * - the regime is AIFS;
 * - the domain is a forecast domain;
 * - the product is not synoptic;
 * - the source increment is missing;
 * - `timespan` is `none`;
 * - exactly one `stattype` block is present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_AIFSFakeDoubleLoopSingleLoop_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind& domainKind) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isAifs                     = input.regime == SimulationRegime::AIFS;
        const bool hasForecastDomain          = domainKind == ProductTimeSpecDomainKind::ForecastDomain;
        const bool isNotSynoptic              = !input.isSynoptic;
        const bool sourceIncrementIsMissing   = !input.timeIncrement.has_value();
        const bool timespanIsNone             = input.timespan.kind == TimespanKind::None;
        const bool hasExactlyOneStattypeBlock = input.stattype.size() == 1;
        const bool requiresFakeDoubleLoop     = input.requiresFakeDoubleLoopSingleLoopRepresentation;

        return isAifs && hasForecastDomain && isNotSynoptic && sourceIncrementIsMissing && timespanIsNone &&
               hasExactlyOneStattypeBlock && requiresFakeDoubleLoop;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `match_AIFSFakeDoubleLoopSingleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Build one pure-AIFS canonical window with missing increment semantics.
 *
 * The complete shape construction remains visible in this builder:
 *
 * 1. assert that the source increment is absent;
 * 2. determine the shape-specific canonical range and processing type;
 * 3. assign missing `timeIncrement` and `typeOfTimeIncrement`;
 * 4. construct and return the one canonical window.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domain Already constructed forecast domain.
 * @return One canonical AIFS statistical window.
 * @throws Mars2GribModelException If AIFS invariants are violated.
 */
inline std::vector<ProductTimeSpecWindow> build_AIFSFakeDoubleLoopSingleLoop_Shape(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool sourceIncrementIsMissing = !input.timeIncrement.has_value();

        if (!sourceIncrementIsMissing) {
            throw Mars2GribModelException("AIFS statistics require timeIncrementInSeconds to be missing",
                                          input.to_json(), Here());
        }


        const auto& stattype             = input.stattype.front();
        const TimeDuration timeRange     = stattype.timeRange;
        const auto statisticalProcessing = stattype.typeOfStatisticalProcessing;

        const bool processingTypesMatch = statisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            throw Mars2GribModelException("AIFS FakeDoubleLoop stattype processing must match the innermost processing",
                                          input.to_json(), Here());
        }


        ProductTimeSpecWindow window{statisticalProcessing, missingTypeOfTimeIncrement(), timeRange,
                                     missingIncrement()};

        return {window};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_AIFSFakeDoubleLoopSingleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

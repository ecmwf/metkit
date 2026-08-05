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
/// @brief Matcher and placeholder leaf builder for a seasonal multi-loop statistic.
///
/// This header is the authoritative implementation of the
/// `SeasonalMultiloop` ProductTimeSpec shape. It deliberately owns both the
/// matcher and the leaf builder for this case.
///
/// The matcher exposes each structural and regime condition through a
/// semantically named Boolean. The builder is intentionally a placeholder until
/// the full seasonal multi-loop construction semantics are implemented.
///
/// @ingroup mars2grib_product_time_spec_shapes
///

#pragma once

#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ForecastLeadUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
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
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind& domainKind) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasSeasonalForecastDomain = domainKind == ProductTimeSpecDomainKind::SeasonalForecastDomain;
        const bool isSeasonal                = product_time_spec::detail::isSeasonal(input);
        const bool isNotSynoptic             = !input.isSynoptic;
        const bool isForecast                = input.simulationType == SimulationType::Forecast;
        const bool hasDurationTimespan       = input.timespan.kind == TimespanKind::Duration;
        const bool hasOuterStattypeBlocks    = !input.stattype.empty();

        return hasSeasonalForecastDomain && isSeasonal && isNotSynoptic && isForecast && hasDurationTimespan &&
               hasOuterStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalMultiloop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Placeholder builder for the seasonal multi-loop shape.
 *
 * @param[in] input Fully normalized ProductTimeSpec input and embedded options.
 * @param[in] domain Already constructed absolute seasonal ProductTimeSpec domain.
 * @return Never returns.
 * @throws Mars2GribModelException Always, because this shape is not implemented yet.
 */
inline std::vector<ProductTimeSpecWindow> build_SeasonalMultiloop_Shape(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)domain;

        throw Mars2GribModelException("SeasonalMultiloop shape is not implemented yet", input.to_json(), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_SeasonalMultiloop_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

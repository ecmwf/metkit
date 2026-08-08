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
/// @file Instant.h
/// @brief Matcher and leaf builder for an instant product.
///
/// This header is the authoritative implementation of the `Instant`
/// ProductTimeSpec shape. It deliberately owns both the matcher and the leaf
/// builder for this case.
///
/// The matcher exposes each structural and regime condition through a
/// semantically named Boolean. The builder keeps the full window-construction
/// flow local: range selection, shape-specific validation, increment
/// resolution, window creation, and ordering are visible here.
///
/// Only genuinely cross-cutting semantics such as `typeOfTimeIncrement` and
/// temporal arithmetic are delegated. All failures are nested in
/// `Mars2GribModelException` with the normalized input snapshot.
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
 * @brief Match an instant product representation accepted by the canonical model.
 *
 * The shape matches when:
 *
 * - the normalized input is not seasonal;
 * - `timespan` is explicitly `none`, or it is missing and the instant
 *   compatibility option allows the missing source representation;
 * - no `stattype` blocks are present;
 * - statistical processing is missing.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domainClassification Previously resolved domain classification; it
 *                                  does not affect instant window topology.
 * @return `true` only when all documented facts hold.
 * @throws Mars2GribModelException If matcher evaluation unexpectedly fails.
 */
inline bool match_Instant_Shape(
    const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isNotSeasonal = !product_time_spec::detail::isSeasonal(input);
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) || timespanIsMissingAndAllowed(input, input.allowMissingTimespanForInstantProduct);
        const bool hasNoStattypeBlocks = input.stattype.empty();
        const bool statisticalProcessingIsMissing =
            input.innerMostTypeOfStatisticalProcessing ==
            metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Missing;

        return isNotSeasonal && hasAcceptedTimespanRepresentation && hasNoStattypeBlocks &&
               statisticalProcessingIsMissing;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_Instant_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Build the canonical zero-length window of an instant product.
 *
 * The builder performs the complete shape construction locally:
 *
 * 1. validate any redundant source increment;
 * 2. create one window with missing statistical processing;
 * 3. assign zero range and missing increment semantics;
 * 4. return the one-element canonical window vector.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] domain Already constructed domain; instant window construction does
 *                   not require its absolute endpoints.
 * @return One canonical instant window.
 * @throws Mars2GribModelException If validation or construction fails.
 */
inline ProductTimeSpecShapeStage1 build_Instant_ShapeStage1(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::validateInstantIncrement;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        validateInstantIncrement(input);

        ProductTimeSpecWindow window{metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Missing,
                                     missingTypeOfTimeIncrement(),
                                     metkit::mars2grib::utils::time_arithmetic::zeroDuration(), missingIncrement()};

        return ProductTimeSpecShapeStage1{{window}};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_Instant_ShapeStage1`", input.to_json(), Here()));
    }
}

inline ProductTimeSpecShape build_Instant_ShapeFinal(
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
            Mars2GribModelException("Failed to execute `build_Instant_ShapeFinal`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

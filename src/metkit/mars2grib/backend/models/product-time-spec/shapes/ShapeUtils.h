// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file ShapeUtils.h
/// @brief Shape-specific policy helpers shared by a narrow subset of ProductTimeSpec shapes.
///
/// This internal header contains helper logic that remains genuinely specific to
/// shape matchers/builders and therefore does not belong in the generic temporal
/// arithmetic or time-increment layers.
///
/// Every function has a documented contract, catches all failures, and rethrows
/// `Mars2GribModelException` directly at the function boundary. Lower-level
/// helpers without `ProductTimeSpecInput` use the location-only constructor.
///
/// @ingroup mars2grib_product_time_spec_detail
///
#pragma once

#include <algorithm>
#include <array>

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Test whether the normalized `timespan` is explicitly `none`.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @return `true` when `timespan.kind` is `None`; otherwise `false`.
 * @throws Mars2GribModelException If evaluation unexpectedly fails.
 */
inline bool timespanIsNone(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return input.timespan.kind == TimespanKind::None;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `timespanIsNone`", input.to_json(), Here()));
    }
}

/**
 * @brief Test whether the normalized `timespan` is missing and accepted by the caller.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] allowMissingTimespan Caller-specific policy deciding whether the
 *            missing representation is accepted for the inspected shape.
 * @return `true` when `timespan.kind` is `Missing` and the caller allows it;
 *         otherwise `false`.
 * @throws Mars2GribModelException If evaluation unexpectedly fails.
 */
inline bool timespanIsMissingAndAllowed(const ProductTimeSpecInput& input, const bool allowMissingTimespan) {
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return input.timespan.kind == TimespanKind::Missing && allowMissingTimespan;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `timespanIsMissingAndAllowed`", input.to_json(), Here()));
    }
}


}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

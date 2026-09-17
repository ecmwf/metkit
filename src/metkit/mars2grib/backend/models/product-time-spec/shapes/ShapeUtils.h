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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
template <class Cntx_t>
inline bool timespanIsNone(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = input.timespan.kind == TimespanKind::None;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `timespanIsNone`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
template <class Cntx_t>
inline bool timespanIsMissingAndAllowed(const ProductTimeSpecInput& input, const bool allowMissingTimespan, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = input.timespan.kind == TimespanKind::Missing && allowMissingTimespan;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `timespanIsMissingAndAllowed`", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}


}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

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

/**
 * @brief Verify whether the innermost statistical processing is allowed at step zero.
 *
 * The operation is accepted when:
 * - the processing type is `Accumulation`; or
 * - the extended zero-length from-start operation set is enabled and the
 *   processing type is `Average`, `Minimum`, or `Maximum`.
 *
 * @param[in] innerTypeOfStatisticalProcessing Innermost processing type to validate.
 * @param[in] allowExtendedSetOfOperationsForZeroLengthFsWindow Whether the
 *            extended zero-length from-start operation set is enabled.
 * @return `true` when the processing type is allowed at step zero; otherwise `false`.
 * @throws Mars2GribModelException If evaluation unexpectedly fails.
 */
inline bool isAllowed_InnerTypeOfStatisticalProcessingAtStepZero(
    const metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing& innerTypeOfStatisticalProcessing,
    const bool allowExtendedSetOfOperationsForZeroLengthFsWindow) {

    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        static constexpr std::array<TypeOfStatisticalProcessing, 3> extendedSetOfOperation = {
            {TypeOfStatisticalProcessing::Average, TypeOfStatisticalProcessing::Minimum,
             TypeOfStatisticalProcessing::Maximum}};

        const bool isAccumulation = innerTypeOfStatisticalProcessing == TypeOfStatisticalProcessing::Accumulation;
        const bool isInExtendedOperationSet = std::any_of(
            extendedSetOfOperation.begin(), extendedSetOfOperation.end(),
            [&innerTypeOfStatisticalProcessing](auto value) { return innerTypeOfStatisticalProcessing == value; });

        if (isAccumulation) {
            return true;
        }

        if (isInExtendedOperationSet) {
            return allowExtendedSetOfOperationsForZeroLengthFsWindow;
        }

        return false;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `isAllowed_InnerTypeOfStatisticalProcessingAtStepZero`", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

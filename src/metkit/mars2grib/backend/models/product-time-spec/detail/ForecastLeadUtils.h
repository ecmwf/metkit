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
/// @file ForecastLeadUtils.h
/// @brief Shared forecast-lead predicates for ProductTimeSpec seasonal semantics.
///
/// This internal header centralizes the small normalized-input predicates used
/// to distinguish step-based forecast products from seasonal forecast products
/// driven by `fcmonth`.
///
/// The helpers in this header are intentionally limited to local Boolean input
/// inspection. They do not classify complete shape or domain cases and they do
/// not perform any temporal arithmetic.
///
/// Every function catches all failures and rethrows `Mars2GribModelException`
/// directly. Functions receiving normalized input attach `input.to_json()`.
///
/// @ingroup mars2grib_product_time_spec_detail
///

#pragma once

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::detail {

///
/// @brief Test whether the normalized input contains an explicit `step` source.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.step` is present, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool hasStep(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return input.step.has_value();
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to test whether ProductTimeSpec input contains step",
                                                       input.to_json(), Here()));
    }
}

///
/// @brief Test whether the normalized input contains an explicit `fcmonth` source.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.marsFcmonth` is present, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool hasFcmonth(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return input.marsFcmonth.has_value();
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to test whether ProductTimeSpec input contains fcmonth", input.to_json(), Here()));
    }
}

///
/// @brief Test whether the normalized input lacks an explicit `step` source.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.step` is absent, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool stepIsMissing(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return !hasStep(input);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to test whether ProductTimeSpec input lacks step", input.to_json(), Here()));
    }
}

///
/// @brief Test whether the normalized input contains an explicit zero-valued `step`.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.step` is present and its length is exactly zero,
///         otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool stepIsZero(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return hasStep(input) && input.step->length == 0;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to test whether ProductTimeSpec input contains a zero step",
                                    input.to_json(), Here()));
    }
}

///
/// @brief Test whether the normalized input contains an explicit positive `step`.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.step` is present and its length is strictly
///         positive, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool stepIsPositive(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return hasStep(input) && input.step->length > 0;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to test whether ProductTimeSpec input contains a positive step",
                                    input.to_json(), Here()));
    }
}

///
/// @brief Test whether the normalized input contains an explicit `fcmonth` source.
///
/// This helper exists as the positive-form companion of `stepIsMissing(...)` so
/// seasonal predicates can read semantically.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.marsFcmonth` is present, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool fcmonthIsPresent(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return hasFcmonth(input);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to test whether ProductTimeSpec input contains fcmonth", input.to_json(), Here()));
    }
}

///
/// @brief Test whether the normalized input uses seasonal forecast lead semantics.
///
/// The seasonal discriminator is intentionally local and exact:
/// - `step` is absent;
/// - `fcmonth` is present.
///
/// No additional simulation-type, shape, or domain facts are evaluated here.
/// Those remain the responsibility of the higher-level matchers that consume
/// this helper.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when the normalized input is seasonal according to the local
///         ProductTimeSpec seasonal discriminator, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool isSeasonal(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        return stepIsMissing(input) && fcmonthIsPresent(input);
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to test whether ProductTimeSpec input is seasonal", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::detail

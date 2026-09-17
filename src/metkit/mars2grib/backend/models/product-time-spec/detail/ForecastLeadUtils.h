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
/// @brief Shared forecast-lead predicates for ProductTimeSpec input-step semantics.
///
/// This internal header centralizes the small normalized-input predicates used
/// to inspect `step` and `fcmonth` presence and value shape.
///
/// The helpers in this header are intentionally limited to local Boolean input
/// inspection. They do not classify complete shape or domain cases and they do
/// not perform any temporal arithmetic.
///
/// Every function catches all failures and rethrows `Mars2GribModelException`
/// directly. Functions receiving normalized input attach `input.to_json(cntx)`.
///
/// @ingroup mars2grib_product_time_spec_detail
///

#pragma once

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
template <class Cntx_t>
inline bool hasStep(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = input.step.has_value();
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to test whether ProductTimeSpec input contains step",
                                                       input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

///
/// @brief Test whether the normalized input contains an explicit `fcmonth` source.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.marsFcmonth` is present, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
template <class Cntx_t>
inline bool hasFcmonth(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = input.marsFcmonth.has_value();
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to test whether ProductTimeSpec input contains fcmonth",
                                                       input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

///
/// @brief Test whether the normalized input lacks an explicit `step` source.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when `input.step` is absent, otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
template <class Cntx_t>
inline bool stepIsMissing(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = !hasStep(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to test whether ProductTimeSpec input lacks step",
                                                       input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
template <class Cntx_t>
inline bool stepIsZero(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = hasStep(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) && input.step->length == 0;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to test whether ProductTimeSpec input contains a zero step", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
template <class Cntx_t>
inline bool stepIsPositive(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = hasStep(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) && input.step->length > 0;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to test whether ProductTimeSpec input contains a positive step", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
template <class Cntx_t>
inline bool fcmonthIsPresent(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        {
            bool result = hasFcmonth(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to test whether ProductTimeSpec input contains fcmonth",
                                                       input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

///
}  // namespace metkit::mars2grib::backend::models::product_time_spec::detail

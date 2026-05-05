/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file timeIncrementInSeconds.h
/// @brief Deduction of the optional `timeIncrementInSeconds` parameter.
///
/// Resolves the `timeIncrementInSeconds` key from the parameter dictionary.
/// The value, when present and strictly positive, denotes the inner time
/// increment (in seconds) of a statistical product (the spacing between
/// the samples consumed by the innermost statistical operation, e.g. the
/// 1-hour spacing of an hourly accumulation).
///
/// Normalization rules:
///   - absent key                     -> `std::nullopt`
///   - present, value `0`             -> `std::nullopt` (legacy normalization)
///   - present, value `> 0`           -> `value`
///   - present, value `< 0`           -> hard error
///
/// Deductions:
/// - extract value from the parameter dictionary
/// - apply normalization rule above
///
/// Deductions do NOT:
/// - infer missing values
/// - apply defaults or fallbacks
/// - validate against external GRIB code tables
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// @section References
/// Consumer:
/// - @ref productTime.h (consumes `timeIncrementInSeconds_opt` to populate
///   the `ProductTime::timeIncrementInSeconds` field)
///
/// Specification:
/// - `deductions/timeProducts.md` §7.8 (`timeIncrementInSeconds` keyword)
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>

// Core deduction includes
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve `timeIncrementInSeconds` as an optional value.
///
/// Reads the `timeIncrementInSeconds` key from the parameter dictionary and
/// applies the normalization rules described in the file-level
/// documentation.
///
/// @tparam MarsDict_t  MARS dictionary type (unused; kept for symmetry with
///                     sibling deductions).
/// @tparam ParDict_t   Parameter dictionary type.
/// @param  mars        MARS dictionary (unused).
/// @param  par         Parameter dictionary; queried for
///                     `timeIncrementInSeconds`.
/// @return `std::optional<long>` holding the strictly-positive seconds value
///         when present, `std::nullopt` otherwise (key absent or value `0`).
/// @throws Mars2GribDeductionException if the key is present and the value
///         is strictly negative, or if any underlying dictionary access
///         throws (rethrown nested with diagnostic context).
///
template <class MarsDict_t, class ParDict_t>
std::optional<long> timeIncrementInSeconds_opt(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        auto lengthOfTimeStepInSeconds_opt = get_opt<long>(par, "timeIncrementInSeconds");

        if (lengthOfTimeStepInSeconds_opt.has_value()) {
            if (lengthOfTimeStepInSeconds_opt.value() < 0) {
                throw Mars2GribDeductionException("`timeIncrementInSeconds` must be > 0 if present", Here());
            }
            else if (lengthOfTimeStepInSeconds_opt.value() == 0) {
                lengthOfTimeStepInSeconds_opt = std::nullopt;
            }
        }

        return lengthOfTimeStepInSeconds_opt;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `timeIncrementInSeconds` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};


///
/// @brief Resolve `timeIncrementInSeconds` or throw if absent.
///
/// Thin wrapper around `timeIncrementInSeconds_opt` that converts a
/// `std::nullopt` result into a hard error. Use when the consumer requires
/// a definite value.
///
/// @tparam MarsDict_t  MARS dictionary type.
/// @tparam ParDict_t   Parameter dictionary type.
/// @param  mars        MARS dictionary (forwarded).
/// @param  par         Parameter dictionary (forwarded).
/// @return The strictly-positive seconds value.
/// @throws Mars2GribDeductionException if the value is absent (key missing
///         or normalized to `std::nullopt`), strictly negative, or if any
///         underlying dictionary access throws.
///
template <class MarsDict_t, class ParDict_t>
long timeIncrementInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        std::optional<long> timeIncrementInSecondsOpt = timeIncrementInSeconds_opt(mars, par);

        if (timeIncrementInSecondsOpt.has_value()) {
            return timeIncrementInSecondsOpt.value();
        }
        else {
            throw Mars2GribDeductionException("`timeIncrementInSeconds` is not defined in Mars/Par dictionary", Here());
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `timeIncrementInSeconds` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

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
/// @file fcmonth.h
/// @brief Public deduction header for the optional raw MARS `fcmonth` key.
///
/// Exposes `resolve_Fcmonth_opt`, the canonical entry point that resolves the
/// raw optional MARS `fcmonth` source into a validated canonical integer
/// forecast lead count expressed in calendar months.
///
/// This deduction owns:
/// - direct `fcmonth` dictionary access;
/// - lexical parsing of integer and textual decimal representations;
/// - local validation that the resolved forecast lead count is strictly
///   positive.
///
/// This deduction does NOT:
/// - materialize a `TimeDuration` value;
/// - validate the relationship between `fcmonth` and any other temporal key;
/// - apply ProductTimeSpec model semantics beyond local source validation.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

namespace detail {

///
/// @brief Validate one canonical integer forecast lead count expressed in months.
///
/// The MARS `fcmonth` keyword is interpreted as a strictly positive integer lead
/// count measured in calendar months.
///
/// @param[in] value Canonical integer forecast lead count.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return `value` unchanged when it is strictly positive.
/// @throws Mars2GribDeductionException if `value` is not strictly positive.
///
inline long checkedPositiveFcmonthLeadCount(long value, const std::string& key) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        if (value <= 0) {
            throw Mars2GribDeductionException("`" + key + "` must be a strictly positive integer", Here());
        }

        return value;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to validate `fcmonth` as a positive forecast lead count", Here()));
    }
}

}  // namespace detail

///
/// @brief Resolve the optional raw MARS `fcmonth` source.
///
/// @section Deduction contract
///   - Reads (MARS): `fcmonth`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `fcmonth` absent -> `std::nullopt`;
/// - numeric `fcmonth` -> validated as a strictly positive integer lead count;
/// - string `fcmonth` -> parsed as a decimal integer, then validated as a
///   strictly positive integer lead count;
/// - malformed, unsupported, or out-of-range values -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `fcmonth`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<long>` containing the canonical integer forecast lead
///         count in months when the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or locally invalid raw `fcmonth` input,
///         with the original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_Fcmonth_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "fcmonth")) {
            return std::nullopt;
        }

        long result = 0;
        if (auto value = get_opt<long>(mars, "fcmonth")) {
            result = detail::checkedPositiveFcmonthLeadCount(*value, "fcmonth");
        }
        else if (auto value = get_opt<std::string>(mars, "fcmonth")) {
            result = detail::checkedPositiveFcmonthLeadCount(detail::parseLongStrict(*value, "fcmonth"), "fcmonth");
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `fcmonth`", Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`fcmonth` resolved from input dictionaries: value='"} + std::to_string(result) + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `fcmonth` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file month.h
/// @brief Public deduction header for the optional raw MARS `month` key.
///
/// Exposes `resolve_Month_opt`, the canonical entry point that resolves the raw
/// optional MARS `month` source into a validated canonical integer month.
///
/// This deduction owns:
/// - direct `month` dictionary access;
/// - lexical parsing of integer and textual month-enum representations;
/// - local validation that the resolved month value is in the enum range
///   `[1,12]`.
///
/// This deduction does NOT:
/// - validate the relationship between `year` and `month`;
/// - construct a date or datetime;
/// - apply any ProductTimeSpec-specific semantics.
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
/// @brief Validate one canonical integer month against the MARS enum range.
///
/// The MARS `month` keyword is an enum whose canonical numeric values are the
/// calendar months `1..12`.
///
/// @param[in] value Canonical integer month value.
/// @param[in] key   Human-readable source-key name used in diagnostics.
/// @return `value` unchanged when it lies in `[1,12]`.
/// @throws Mars2GribDeductionException if `value` lies outside `[1,12]`.
///
inline long checkedMonthEnumValue(long value, const std::string& key) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (value < 1 || value > 12) {
        throw Mars2GribDeductionException("`" + key + "` must be in [1,12]", Here());
    }
    return value;
}

}  // namespace detail

///
/// @brief Resolve the optional raw MARS `month` source.
///
/// @section Deduction contract
///   - Reads (MARS): `month`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `month` absent -> `std::nullopt`;
/// - numeric `month` -> validated as an enum value in `[1,12]`;
/// - string `month` -> parsed as one supported month alias or decimal integer,
///   then validated as an enum value in `[1,12]`;
/// - malformed, unsupported, or out-of-range values -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `month`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<long>` containing the canonical integer month when
///         the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or out-of-range raw `month` input, with
///         the original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_Month_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "month")) {
            return std::nullopt;
        }

        long result = 0;
        if (auto value = get_opt<long>(mars, "month")) {
            result = detail::checkedMonthEnumValue(*value, "month");
        }
        else if (auto value = get_opt<std::string>(mars, "month")) {
            result = detail::checkedMonthEnumValue(detail::parseMonthEnum(*value, "month"), "month");
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `month`", Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`month` resolved from input dictionaries: value='"} + std::to_string(result) + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `month` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

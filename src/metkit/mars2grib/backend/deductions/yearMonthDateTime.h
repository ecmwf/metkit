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
/// @file yearMonthDateTime.h
/// @brief Public deduction header for `yearMonthDateTime`.
///
/// Exposes `resolve_YearMonthDateTime_opt` and
/// `resolve_YearMonthDateTime_or_throw`, the canonical entry points that resolve
/// the optional year/month-based datetime source from MARS input dictionaries.
///
/// This deduction owns:
/// - direct `year` / `month` dictionary access;
/// - lexical parsing of supported integer representations;
/// - local validation of the pair-level source invariant;
/// - construction of the first day of the month at `00:00:00`.
///
/// This deduction does NOT:
/// - inherit values from other temporal sources;
/// - classify ProductTimeSpec semantics;
/// - construct the final ProductTimeSpec model.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve `yearMonthDateTime` as an optional value.
///
/// @section Deduction contract
///   - Reads (MARS): `year`, `month`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `year` absent and `month` absent -> `std::nullopt`;
/// - exactly one of `year` / `month` present -> hard error;
/// - both present -> `DateTime(Date(year, month, 1), 00:00:00)`.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `year` and `month`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<eckit::DateTime>` containing the first-day-of-month
///         datetime when both keys are present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, incomplete, or invalid `year` / `month`
///         input, with the original cause attached via
///         `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<eckit::DateTime> resolve_YearMonthDateTime_opt(const MarsDict_t& mars, const ParDict_t& par,
                                                             const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        const bool hasYear = has(mars, "year");
        const bool hasMonth = has(mars, "month");

        if (!hasYear && !hasMonth) {
            return std::nullopt;
        }
        if (hasYear != hasMonth) {
            throw Mars2GribDeductionException("`year` and `month` must be both present or both absent", Here());
        }

        long year = 0;
        if (auto value = get_opt<long>(mars, "year")) {
            year = *value;
        } else if (auto value = get_opt<std::string>(mars, "year")) {
            year = detail::parseLongStrict(*value, "year");
        } else {
            throw Mars2GribDeductionException("Unsupported type for `year`", Here());
        }

        long month = 0;
        if (auto value = get_opt<long>(mars, "month")) {
            month = *value;
        } else if (auto value = get_opt<std::string>(mars, "month")) {
            month = detail::parseLongStrict(*value, "month");
        } else {
            throw Mars2GribDeductionException("Unsupported type for `month`", Here());
        }

        if (month < 1 || month > 12) {
            throw Mars2GribDeductionException("`month` must be in [1,12]", Here());
        }

        eckit::Date date{};
        try {
            date = eckit::Date(year, month, 1);
        } catch (...) {
            throw Mars2GribDeductionException("Invalid `year`/`month` datetime source", Here());
        }

        const eckit::DateTime result{date, eckit::Time(0, 0, 0)};
        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`yearMonthDateTime` resolved from input dictionaries: value='"} + result.iso(true) + "'";
        }());
        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `yearMonthDateTime` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `yearMonthDateTime` or throw if absent.
///
/// Thin wrapper around `resolve_YearMonthDateTime_opt` that converts
/// `std::nullopt` into a hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `year` and `month`.
/// @param[in] par   Parameter dictionary (forwarded).
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The year/month-based datetime.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, unsupported, incomplete, or
///         invalid; failures are wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
eckit::DateTime resolve_YearMonthDateTime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const auto result = resolve_YearMonthDateTime_opt(mars, par, opt);
        if (result.has_value()) {
            return *result;
        }
        throw Mars2GribDeductionException("`yearMonthDateTime` is not defined in the Mars dictionary", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `yearMonthDateTime` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

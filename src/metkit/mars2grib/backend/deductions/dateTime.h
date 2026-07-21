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
/// @file dateTime.h
/// @brief Public deduction header for `dateTime`.
///
/// Exposes `resolve_DateTime_opt` and `resolve_DateTime_or_throw`, the
/// canonical entry points that resolve the optional direct initial-conditions
/// datetime source from MARS input dictionaries.
///
/// This deduction owns:
/// - direct `date` / `time` dictionary access;
/// - lexical parsing of the supported MARS date/time representations;
/// - local direct-source validation, including rejection of `time` without
///   `date`.
///
/// This deduction does NOT:
/// - default from other temporal sources;
/// - classify ProductTimeSpec anchor, shape, or increment semantics;
/// - construct the final ProductTimeSpec model.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/dateTimeHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve `dateTime` as an optional direct initial-conditions datetime source.
///
/// @section Deduction contract
///   - Reads (MARS): `date`, `time`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `date` absent and `time` absent -> `std::nullopt`;
/// - `date` present and `time` absent -> `DateTime(date, 00:00:00)`;
/// - `date` present and `time` present -> `DateTime(date, time)`;
/// - `time` present without `date` -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `date` and `time`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<eckit::DateTime>` containing the direct
///         initial-conditions datetime when `date` is present, `std::nullopt`
///         otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or locally contradictory direct
///         initial-conditions input, with the original cause attached via
///         `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<eckit::DateTime> resolve_DateTime_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        const bool hasDate = has(mars, "date");
        const bool hasTime = has(mars, "time");

        if (!hasDate && !hasTime) {
            return std::nullopt;
        }
        if (!hasDate && hasTime) {
            throw Mars2GribDeductionException("`time` is present without `date`", Here());
        }

        eckit::Date date{};
        if (auto value = get_opt<long>(mars, "date")) {
            date = detail::parseDateLong(*value, "date");
        } else if (auto value = get_opt<std::string>(mars, "date")) {
            date = detail::parseDateString(*value, "date");
        } else {
            throw Mars2GribDeductionException("Unsupported type for `date`", Here());
        }

        eckit::Time time{0, 0, 0};
        if (hasTime) {
            if (auto value = get_opt<long>(mars, "time")) {
                time = detail::parseTimeLong(*value, "time");
            } else if (auto value = get_opt<std::string>(mars, "time")) {
                time = detail::parseTimeString(*value, "time");
            } else {
                throw Mars2GribDeductionException("Unsupported type for `time`", Here());
            }
        }

        const eckit::DateTime result{date, time};
        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`dateTime` resolved from input dictionaries: value='"} + result.iso(true) + "'";
        }());
        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `dateTime` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `dateTime` or throw if absent.
///
/// Thin wrapper around `resolve_DateTime_opt` that converts `std::nullopt` into
/// a hard error. Use when the consumer requires a definite direct
/// initial-conditions datetime source.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `date` and `time`.
/// @param[in] par   Parameter dictionary (forwarded).
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The direct initial-conditions datetime.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, unsupported, or locally
///         contradictory; failures are wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
eckit::DateTime resolve_DateTime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const auto result = resolve_DateTime_opt(mars, par, opt);
        if (result.has_value()) {
            return *result;
        }
        throw Mars2GribDeductionException("`dateTime` is not defined in the Mars dictionary", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `dateTime` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

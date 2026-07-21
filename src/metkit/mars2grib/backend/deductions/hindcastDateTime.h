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
/// @file hindcastDateTime.h
/// @brief Public deduction header for `hindcastDateTime`.
///
/// Exposes `resolve_HindcastDateTime_opt` and
/// `resolve_HindcastDateTime_or_throw`, the canonical entry points that resolve
/// the optional hindcast datetime source from MARS input dictionaries.
/// When present, `hindcastDateTime` is the direct `labelDateTime` source used
/// by ProductTimeSpec anchor materialization.
///
/// This deduction owns:
/// - direct `hdate` dictionary access;
/// - lexical parsing of the supported MARS date representations;
/// - construction of `DateTime(hdate, 00:00:00)`.
///
/// This deduction does NOT:
/// - infer missing hindcast datetimes from other temporal sources;
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
#include "metkit/mars2grib/backend/deductions/detail/dateTimeHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve `hindcastDateTime` as an optional direct label datetime source.
///
/// @section Deduction contract
///   - Reads (MARS): `hdate`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `hdate` absent -> `std::nullopt`;
/// - `hdate` present -> `DateTime(hdate, 00:00:00)`.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `hdate`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<eckit::DateTime>` containing the direct label
///         datetime when `hdate` is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed or unsupported `hdate` input, with the original cause
///         attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<eckit::DateTime> resolve_HindcastDateTime_opt(const MarsDict_t& mars, const ParDict_t& par,
                                                            const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "hdate")) {
            return std::nullopt;
        }

        eckit::Date date{};
        if (auto value = get_opt<long>(mars, "hdate")) {
            date = detail::parseDateLong(*value, "hdate");
        } else if (auto value = get_opt<std::string>(mars, "hdate")) {
            date = detail::parseDateString(*value, "hdate");
        } else {
            throw Mars2GribDeductionException("Unsupported type for `hdate`", Here());
        }

        const eckit::DateTime result{date, eckit::Time(0, 0, 0)};
        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`hindcastDateTime` resolved from input dictionaries: value='"} + result.iso(true) + "'";
        }());
        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `hindcastDateTime` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `hindcastDateTime` or throw if absent.
///
/// Thin wrapper around `resolve_HindcastDateTime_opt` that converts
/// `std::nullopt` into a hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `hdate`.
/// @param[in] par   Parameter dictionary (forwarded).
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The direct label datetime resolved from `hdate`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, or unsupported; failures are
///         wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
eckit::DateTime resolve_HindcastDateTime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const auto result = resolve_HindcastDateTime_opt(mars, par, opt);
        if (result.has_value()) {
            return *result;
        }
        throw Mars2GribDeductionException("`hindcastDateTime` is not defined in the Mars dictionary", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `hindcastDateTime` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

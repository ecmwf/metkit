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
/// @file step.h
/// @brief Public deduction header for `step`.
///
/// Exposes `resolve_Step_opt` and `resolve_Step_or_throw`, the canonical entry
/// points that resolve the optional normalized MARS `step` duration.
///
/// This deduction owns:
/// - direct `step` dictionary access;
/// - lexical parsing of numeric and unit-suffixed step forms;
/// - local support-domain validation of the resolved duration;
/// - materialization of the deduction-local `TimeDuration` output.
///
/// This deduction does NOT:
/// - infer step from other temporal sources;
/// - classify ProductTimeSpec semantics;
/// - construct the final ProductTimeSpec model.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/backend/deductions/detail/timeDurationHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {


///
/// @brief Resolve `step` as an optional normalized duration.
///
/// @section Deduction contract
///   - Reads (MARS): `step`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `step` absent -> `std::nullopt`;
/// - numeric `step` -> interpreted as hours;
/// - string `step` -> parsed using the supported duration language;
/// - resolved duration must be non-negative;
/// - positive sub-hourly or non-hour-aligned durations are rejected.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `step`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<TimeDuration>` containing the normalized step when
///         the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or locally invalid step input, with the
///         original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<TimeDuration> resolve_Step_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "step")) {
            return std::nullopt;
        }

        long seconds = 0;
        if (auto value = get_opt<std::string>(mars, "step")) {
            seconds = detail::parseDurationStringSeconds(*value, "step");
        } else if (auto value = get_opt<long>(mars, "step")) {
            seconds = detail::checkedHoursToSeconds(*value, "step");
        } else {
            throw Mars2GribDeductionException("Unsupported type for `step`", Here());
        }

        if (seconds < 0) {
            throw Mars2GribDeductionException("`step` must be non-negative", Here());
        }
        if (seconds > 0 && seconds % 3600L != 0) {
            throw Mars2GribDeductionException(
                "Positive sub-hourly or non-hour-aligned `step` is recognized but unsupported", Here());
        }

        const TimeDuration result = detail::canonicalElapsedDuration(seconds, "step");
        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`step` resolved from input dictionaries: value='"} + std::to_string(result.length) +
                   "' unit='" + tables::enum2name_TimeUnit_or_throw(result.unit) + "'";
        }());
        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `step` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `step` or throw if absent.
///
/// Thin wrapper around `resolve_Step_opt` that converts `std::nullopt` into a
/// hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `step`.
/// @param[in] par   Parameter dictionary (forwarded).
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The normalized step duration.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, unsupported, or locally invalid;
///         failures are wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
TimeDuration resolve_Step_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const auto result = resolve_Step_opt(mars, par, opt);
        if (result.has_value()) {
            return *result;
        }
        throw Mars2GribDeductionException("`step` is not defined in the Mars dictionary", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `step` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

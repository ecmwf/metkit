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
/// @file timeIncrement.h
/// @brief Public deduction header for `timeIncrement`.
///
/// Exposes `resolve_TimeIncrement_opt` and `resolve_TimeIncrement_or_throw`,
/// the canonical entry points that resolve the optional normalized parameter-
/// side time increment duration.
///
/// This deduction owns:
/// - direct `timeIncrementInSeconds` dictionary access;
/// - lexical parsing of numeric and string integer representations;
/// - local positivity validation;
/// - materialization of the deduction-local `TimeDuration` output.
///
/// This deduction is intentionally independent from the legacy
/// `timeIncrementInSeconds.h` deduction.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include <optional>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

namespace detail {

///
/// @brief Canonicalize one strictly positive time increment expressed in seconds.
///
/// Positive whole-hour values are represented in hours. Other strictly positive
/// values are represented in seconds.
///
/// @param[in] seconds Elapsed increment in seconds.
/// @return Canonical time increment duration.
/// @throws Mars2GribDeductionException if `seconds <= 0`.
///
template <class Cntx_t>
inline TimeDuration canonicalTimeIncrementDuration(long seconds, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (seconds <= 0) {
        throw Mars2GribDeductionException("`timeIncrementInSeconds` must be strictly positive when present", Here());
    }
    if (seconds % 3600L == 0) {
        {
            TimeDuration result = TimeDuration{seconds / 3600L, tables::TimeUnit::Hour};
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    {
        TimeDuration result = TimeDuration{seconds, tables::TimeUnit::Second};
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

}  // namespace detail

///
/// @brief Resolve `timeIncrement` as an optional normalized duration.
///
/// @section Deduction contract
///   - Reads (MARS): none (signature-only, reserved)
///   - Reads (par):  `timeIncrementInSeconds`
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `timeIncrementInSeconds` absent -> `std::nullopt`;
/// - present valid positive value -> normalized `TimeDuration`;
/// - present zero, negative, malformed, or unsupported value -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type (currently unused).
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary (signature-only).
/// @param[in] par   Parameter dictionary providing `timeIncrementInSeconds`.
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<TimeDuration>` containing the normalized time
///         increment when the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed or locally invalid parameter input, with the original
///         cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
std::optional<TimeDuration> resolve_TimeIncrement_opt(const MarsDict_t& mars, const ParDict_t& par,
                                                      const OptDict_t& opt, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)mars;
    (void)opt;

    try {
        if (!has(par, "timeIncrementInSeconds", metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
            {
                std::optional<TimeDuration> result = std::nullopt;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }

        long seconds = 0;
        if (auto value = get_opt<long>(par, "timeIncrementInSeconds", metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
            seconds = *value;
        }
        else if (auto value = get_opt<std::string>(par, "timeIncrementInSeconds", metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
            seconds = detail::parseLongStrict(*value, "timeIncrementInSeconds", metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `timeIncrementInSeconds`", Here());
        }

        const TimeDuration result = detail::canonicalTimeIncrementDuration(seconds, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`timeIncrement` resolved from input dictionaries: value='"} +
                   std::to_string(result.length) + "' unit='" + tables::enum2name_TimeUnit_or_throw(result.unit, cntx) + "'";
        }());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `timeIncrement` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `timeIncrement` or throw if absent.
///
/// Thin wrapper around `resolve_TimeIncrement_opt` that converts `std::nullopt`
/// into a hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary (forwarded).
/// @param[in] par   Parameter dictionary providing `timeIncrementInSeconds`.
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The normalized time increment duration.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, unsupported, or locally invalid;
///         failures are wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
TimeDuration resolve_TimeIncrement_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const std::optional<TimeDuration> resolved =
            resolve_TimeIncrement_opt(mars, par, opt,
                                      metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        if (resolved.has_value()) {
            {
                TimeDuration result = *resolved;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
        throw Mars2GribDeductionException("`timeIncrement` is not defined in the Par dictionary", Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `timeIncrement` from Par dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

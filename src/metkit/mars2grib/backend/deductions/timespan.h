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
/// @file timespan.h
/// @brief Public deduction header for `timespan`.
///
/// Exposes `resolve_Timespan_opt` and `resolve_Timespan_or_throw`, the
/// canonical entry points that resolve the normalized MARS `timespan`
/// representation from input dictionaries.
///
/// This deduction owns:
/// - direct `timespan` dictionary access;
/// - classification of the supported `timespan` source language into
///   `Missing`, `Duration`, `None`, or `FromStart`;
/// - local lexical validation of string and integer representations;
/// - application of the compatibility option
///   `allowNonEnumeratedPositiveIntegerTimespanHours`.
///
/// This deduction does NOT:
/// - classify ProductTimeSpec shape semantics beyond the local source form;
/// - combine `timespan` with `stattype` or `step`;
/// - construct the final ProductTimeSpec model.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/backend/deductions/detail/timeDurationHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

namespace detail {

///
/// @brief Test whether an integer-hour timespan belongs to the supported set.
///
/// @param[in] hours Integer-hour duration.
/// @return `true` when `hours` is language-enumerated for the supported domain.
///
inline bool isSupportedTimespanHours(long hours) {
    constexpr std::array<long, 12> supported{1, 3, 6, 12, 18, 24, 48, 72, 120, 168, 240, 360};
    return std::find(supported.begin(), supported.end(), hours) != supported.end();
}

///
/// @brief Test whether a textual timespan belongs to the supported set.
///
/// @param[in] value Lowercase `timespan` token.
/// @return `true` when `value` is language-enumerated for the supported domain.
///
inline bool isSupportedTimespanString(const std::string& value) {
    constexpr std::array<std::string_view, 12> supported{"1h",  "3h",  "6h",   "12h",  "18h",  "24h",
                                                         "48h", "72h", "120h", "168h", "240h", "360h"};
    return std::find(supported.begin(), supported.end(), value) != supported.end();
}

///
/// @brief Test whether a textual timespan is recognized but unsupported.
///
/// @param[in] value Lowercase `timespan` token.
/// @return `true` when `value` is known but intentionally outside the current
///         supported domain.
///
inline bool isRecognizedUnsupportedTimespan(const std::string& value) {
    return value == "inst" || value == "instantaneous" || value == "10m" || value == "15m" || value == "20m" ||
           value == "30m";
}


///
/// @brief Read the integer-hour compatibility policy for `timespan`.
///
/// The option accepts boolean, `0`/`1`, or boolean-like string forms.
///
/// @tparam OptDict_t Options dictionary type.
/// @param[in] opt Options dictionary.
/// @return The resolved compatibility flag.
/// @throws Mars2GribDeductionException if the option is present but not a valid
///         boolean representation.
///
template <class OptDict_t>
inline bool allowNonEnumeratedPositiveIntegerTimespanHours(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    constexpr std::string_view key = "allowNonEnumeratedPositiveIntegerTimespanHours";
    if (!has(opt, key)) {
        return metkit::mars2grib::defaults::allowNonEnumeratedPositiveIntegerTimespanHours;
    }
    if (auto value = get_opt<bool>(opt, key)) {
        return *value;
    }

    throw Mars2GribDeductionException("Invalid boolean option `allowNonEnumeratedPositiveIntegerTimespanHours`",
                                      Here());
}

}  // namespace detail

///
/// @brief Resolve `timespan` as an optional normalized source representation.
///
/// @section Deduction contract
///   - Reads (MARS): `timespan`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  `allowNonEnumeratedPositiveIntegerTimespanHours`
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `timespan` absent -> `std::nullopt`;
/// - `timespan="none"` -> `Timespan{None, nullopt}`;
/// - `timespan` from-start aliases -> `Timespan{FromStart, nullopt}`;
/// - supported duration-valued `timespan` -> `Timespan{Duration, duration}`;
/// - recognized but unsupported or invalid values -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `timespan`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary providing the integer-hour compatibility
///                  policy.
///
/// @return `std::optional<Timespan>` containing the normalized source
///         representation when `timespan` is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or locally invalid `timespan` input, with
///         the original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<Timespan> resolve_Timespan_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;

    try {
        if (!has(mars, "timespan")) {
            return std::nullopt;
        }

        Timespan result;
        if (auto value = get_opt<std::string>(mars, "timespan")) {
            const std::string normalized = detail::lower(*value);
            if (normalized == "none") {
                result.kind = TimespanKind::None;
            }
            else if (normalized == "fs" || normalized == "from-start" || normalized == "fromstart") {
                result.kind = TimespanKind::FromStart;
            }
            else {
                if (detail::isRecognizedUnsupportedTimespan(normalized)) {
                    throw Mars2GribDeductionException("Recognized but unsupported `timespan`: '" + *value + "'",
                                                      Here());
                }
                if (!detail::isSupportedTimespanString(normalized)) {
                    throw Mars2GribDeductionException(
                        "String `timespan` is not in the supported language-defined set: '" + *value + "'", Here());
                }

                const long seconds = detail::parseDurationStringSeconds(normalized, "timespan");
                if (seconds <= 0 || seconds % 3600L != 0) {
                    throw Mars2GribDeductionException(
                        "String `timespan` is not in the supported language-defined set: '" + *value + "'", Here());
                }

                result.kind     = TimespanKind::Duration;
                result.duration = detail::canonicalElapsedDuration(seconds, "timespan");
            }
        }
        else if (auto value = get_opt<long>(mars, "timespan")) {
            if (*value < 1) {
                throw Mars2GribDeductionException("Integer `timespan` must be >= 1 hour", Here());
            }
            if (!detail::allowNonEnumeratedPositiveIntegerTimespanHours(opt) &&
                !detail::isSupportedTimespanHours(*value)) {
                throw Mars2GribDeductionException(
                    "Integer `timespan` is not language-enumerated and compatibility option is disabled", Here());
            }

            result.kind     = TimespanKind::Duration;
            result.duration = TimeDuration{*value, tables::TimeUnit::Hour};
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `timespan`", Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string msg = "`timespan` resolved from input dictionaries: kind='";
            switch (result.kind) {
                case TimespanKind::Missing:
                    msg += "missing";
                    break;
                case TimespanKind::Duration:
                    msg += "duration";
                    break;
                case TimespanKind::None:
                    msg += "none";
                    break;
                case TimespanKind::FromStart:
                    msg += "from-start";
                    break;
            }
            msg += "'";
            if (result.duration.has_value()) {
                msg += " length='" + std::to_string(result.duration->length) + "' unit='" +
                       tables::enum2name_TimeUnit_or_throw(result.duration->unit) + "'";
            }
            return msg;
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `timespan` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `timespan` or throw if absent.
///
/// Thin wrapper around `resolve_Timespan_opt` that converts `std::nullopt` into
/// a hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `timespan`.
/// @param[in] par   Parameter dictionary (forwarded).
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The normalized `timespan` source representation.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, unsupported, or locally invalid;
///         failures are wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
Timespan resolve_Timespan_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const auto result = resolve_Timespan_opt(mars, par, opt);
        if (result.has_value()) {
            return *result;
        }
        else {
            return Timespan{TimespanKind::Missing, std::nullopt};
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `timespan` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

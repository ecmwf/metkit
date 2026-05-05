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
/// @file productTime.h
/// @brief Public deduction header for `ProductTime`.
///
/// Exposes `resolve_ProductTime_or_throw`, the canonical entry point that
/// produces a `ProductTime` from MARS / par / opt input dictionaries.
///
/// All temporal consumers (`referenceTime`, `pointInTime`, `statistics`)
/// MUST obtain their temporal data exclusively from a `ProductTime`
/// produced by this function. They MUST NOT reinterpret raw MARS keys
/// (`date`, `time`, `hdate`, `htime`, `fcyear`, `fcmonth`, `step`,
/// `timespan`, `stattype`) independently.
///
/// The implementation detail (types, factory, helpers) lives in
/// `deductions/detail/ProductTime.h` (UpperCamelCase initial per §20.1
/// because the file is type-primary). The shared `stattype` parser lives
/// in `deductions/detail/StatType.h` (§22). The shared `StatisticalWindow`
/// type lives in `deductions/detail/StatisticalWindow.h` (§21).
///
/// See `deductions/timeProducts.md` for the full normative specification.
///
/// @section References
/// Concept consumers:
///   - referenceTimeEncoding.h
///   - pointInTimeEncoding.h
///   - statisticsEncoding.h
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

// System includes
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

// eckit
#include "eckit/exception/Exceptions.h"
#include "eckit/types/DateTime.h"

// Project utilities (must precede timeIncrementInSeconds.h, which uses
// dict_traits but does not include the corresponding header itself).
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

// Detail
#include "metkit/mars2grib/backend/deductions/detail/ProductTime.h"
#include "metkit/mars2grib/backend/deductions/detail/StatType.h"

// Sibling deductions
#include "metkit/mars2grib/backend/deductions/timeIncrementInSeconds.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the canonical `ProductTime` for one MARS product.
///
/// @section Deduction contract
///   - Reads (MARS): `date`, `time`, `hdate`, `htime`, `fcyear`, `fcmonth`,
///                   `step`, `timespan`, `stattype`
///   - Reads (par):  `timeIncrementInSeconds` (via `timeIncrementInSeconds_opt`)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution proceeds in two stages:
///   1. **Resolver** (this function): reads the input dictionaries and
///      normalizes them into a `ProductTimeInput`.
///   2. **Factory** (`detail::make_ProductTime_or_throw`): validates all
///      invariants and returns the immutable `ProductTime`.
///
/// On success, exactly one composite RESOLVE log line is emitted listing
/// every resolved field in a stable, greppable form (§12).
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing temporal keys.
/// @param[in] par   Parameter dictionary providing `timeIncrementInSeconds`.
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return The resolved, immutable `ProductTime`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on any rule violation (§10.x), with the original cause attached
///         via `std::throw_with_nested`.
///
/// @note This function is thread-safe given thread-safe dictionary reads
///       (§14).
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
detail::ProductTime resolve_ProductTime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    // Suppress "unused parameter" warning while preserving the documented
    // signature (§8: opt is reserved).
    (void)opt;

    try {

        // =========================================================
        // §7.1 / §7.2: simulationDateTime from (date, time) or
        //                  defaulted from (fcyear, fcmonth)
        // =========================================================

        const bool hasDate    = has(mars, "date");
        const bool hasTime    = has(mars, "time");
        const bool hasFcYear  = has(mars, "fcyear");
        const bool hasFcMonth = has(mars, "fcmonth");

        eckit::DateTime simulationDateTime;
        if (hasDate && hasTime) {
            const long marsDate = get_or_throw<long>(mars, "date");
            const long marsTime = get_or_throw<long>(mars, "time");
            simulationDateTime  = eckit::DateTime(detail::convert_YYYYMMDD2Date_or_throw(marsDate),
                                                  detail::convert_hhmmss2Time_or_throw(marsTime));
        }
        else if (!hasDate && !hasTime && hasFcYear && hasFcMonth) {
            // R2 default: simulationDateTime := DateTime(fcyear, fcmonth, 1, 00:00:00).
            const long fcYear  = get_or_throw<long>(mars, "fcyear");
            const long fcMonth = get_or_throw<long>(mars, "fcmonth");
            try {
                simulationDateTime = eckit::DateTime(eckit::Date(fcYear, fcMonth, 1), eckit::Time(0, 0, 0));
            }
            catch (const eckit::Exception& e) {
                throw Mars2GribDeductionException(
                    "Invalid (fcyear, fcmonth) for default simulationDateTime: " + std::string(e.what()), Here());
            }
        }
        else {
            // §10.1: date or time missing without an applicable fallback.
            throw Mars2GribDeductionException(std::string("ProductTime invariant violated [§10.1]: ") +
                                                  "missing 'date'/'time' without (fcyear,fcmonth) fallback. " +
                                                  "has(date)=" + (hasDate ? "true" : "false") +
                                                  ", has(time)=" + (hasTime ? "true" : "false") +
                                                  ", has(fcyear)=" + (hasFcYear ? "true" : "false") +
                                                  ", has(fcmonth)=" + (hasFcMonth ? "true" : "false"),
                                              Here());
        }

        // =========================================================
        // §7.3: simulatedDateTime from (hdate, htime)
        // =========================================================

        const bool hasHdate = has(mars, "hdate");
        const bool hasHtime = has(mars, "htime");

        std::optional<eckit::DateTime> simulatedDateTime;
        if (!hasHdate && !hasHtime) {
            // simulatedDateTime defaults to simulationDateTime in the factory.
        }
        else if (hasHdate && !hasHtime) {
            const long marsHdate = get_or_throw<long>(mars, "hdate");
            simulatedDateTime =
                eckit::DateTime(detail::convert_YYYYMMDD2Date_or_throw(marsHdate), eckit::Time(0, 0, 0));
        }
        else if (hasHdate && hasHtime) {
            const long marsHdate = get_or_throw<long>(mars, "hdate");
            const long marsHtime = get_or_throw<long>(mars, "htime");
            simulatedDateTime    = eckit::DateTime(detail::convert_YYYYMMDD2Date_or_throw(marsHdate),
                                                   detail::convert_hhmmss2Time_or_throw(marsHtime));
        }
        else {
            // §10.2: htime present without hdate.
            throw Mars2GribDeductionException("ProductTime invariant violated [§10.2]: 'htime' present without 'hdate'",
                                              Here());
        }

        // =========================================================
        // §7.4: referenceDateTime from (fcyear, fcmonth)
        // =========================================================

        std::optional<eckit::DateTime> referenceDateTime;
        if (!hasFcYear && !hasFcMonth) {
            // referenceDateTime defaults to simulatedDateTime in the factory.
        }
        else if (hasFcYear && hasFcMonth) {
            const long fcYear  = get_or_throw<long>(mars, "fcyear");
            const long fcMonth = get_or_throw<long>(mars, "fcmonth");
            try {
                referenceDateTime = eckit::DateTime(eckit::Date(fcYear, fcMonth, 1), eckit::Time(0, 0, 0));
            }
            catch (const eckit::Exception& e) {
                throw Mars2GribDeductionException(
                    "Invalid (fcyear, fcmonth) for referenceDateTime: " + std::string(e.what()), Here());
            }
        }
        else {
            // §10.3: exactly one of fcyear / fcmonth present.
            throw Mars2GribDeductionException(std::string("ProductTime invariant violated [§10.3]: ") +
                                                  "exactly one of 'fcyear'/'fcmonth' present. " +
                                                  "has(fcyear)=" + (hasFcYear ? "true" : "false") +
                                                  ", has(fcmonth)=" + (hasFcMonth ? "true" : "false"),
                                              Here());
        }

        // =========================================================
        // §7.5: stepInSeconds from step (default 0 if missing)
        // =========================================================

        long stepInSeconds = 0;
        if (has(mars, "step")) {
            // MARS may expose 'step' either as a long (legacy hours) or as a
            // string (suffixed). Prefer a string read so the parser handles
            // both bare-numeric and unit-suffixed forms uniformly.
            std::optional<std::string> stepStr = get_opt<std::string>(mars, "step");
            if (stepStr.has_value()) {
                stepInSeconds = detail::toSeconds_or_throw(stepStr.value());
            }
            else {
                // Fallback: numeric step interpreted as hours (§7.5 bare-numeric rule).
                const long marsStep = get_or_throw<long>(mars, "step");
                stepInSeconds       = marsStep * 3600L;
            }
        }

        // =========================================================
        // §7.6: timespan
        // =========================================================

        detail::TimespanKind timespanKind = detail::TimespanKind::Missing;
        detail::StatisticalWindow timespan{};

        if (has(mars, "timespan")) {
            std::optional<std::string> tsStr = get_opt<std::string>(mars, "timespan");
            if (tsStr.has_value()) {
                if (tsStr.value() == "none") {
                    timespanKind = detail::TimespanKind::None;
                }
                else {
                    timespanKind   = detail::TimespanKind::Duration;
                    timespan.unit  = tables::TimeUnit::Second;
                    timespan.count = detail::toSeconds_or_throw(tsStr.value());
                }
            }
            else {
                // Numeric-only timespan: interpret as hours per §7.6 (→ §7.5).
                const long tsNum = get_or_throw<long>(mars, "timespan");
                timespanKind     = detail::TimespanKind::Duration;
                timespan.unit    = tables::TimeUnit::Second;
                timespan.count   = tsNum * 3600L;
            }
        }

        // =========================================================
        // §7.7: stattype → stattypeWindows (period part only)
        //
        // Parser is shared with resolve_TypeOfStatisticalProcessing_or_throw
        // (§22). This deduction consumes only the `timeWindow` field of
        // each parsed block; the `typeOfStatisticalProcessing` field is
        // consumed by the sibling deduction.
        // =========================================================

        std::array<detail::StatisticalWindow, detail::maxStatisticalWindows> stattypeWindows{};
        std::size_t stattypeWindowCount = 0;

        if (has(mars, "stattype")) {
            const std::string statTypeVal                         = get_or_throw<std::string>(mars, "stattype");
            const std::vector<detail::ParsedStatTypeBlock> blocks = detail::parse_StatType_or_throw(statTypeVal);

            if (blocks.size() > detail::maxStatisticalWindows) {
                throw Mars2GribDeductionException(
                    "ProductTime invariant violated [§10.15]: stattype yields " + std::to_string(blocks.size()) +
                        " block(s) > maxStatisticalWindows (" + std::to_string(detail::maxStatisticalWindows) + ")",
                    Here());
            }

            for (std::size_t i = 0; i < blocks.size(); ++i) {
                stattypeWindows[i] = blocks[i].timeWindow;
            }
            stattypeWindowCount = blocks.size();
        }

        // =========================================================
        // §7.8: timeIncrementInSeconds (par)
        // =========================================================

        const std::optional<long> tInc = timeIncrementInSeconds_opt(mars, par);

        // =========================================================
        // Assemble the input bundle and call the factory.
        // =========================================================

        detail::ProductTimeInput input;
        input.simulationDateTime     = simulationDateTime;
        input.simulatedDateTime      = simulatedDateTime;
        input.referenceDateTime      = referenceDateTime;
        input.stepInSeconds          = stepInSeconds;
        input.timespanKind           = timespanKind;
        input.timespan               = timespan;
        input.stattypeWindows        = stattypeWindows;
        input.stattypeWindowCount    = stattypeWindowCount;
        input.timeIncrementInSeconds = tInc;

        detail::ProductTime pt = detail::make_ProductTime_or_throw(input);

        // =========================================================
        // §12: composite RESOLVE log line (success path only)
        // =========================================================

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string msg = "`ProductTime` resolved from input dictionaries: ";
            msg += "simulationDateTime='" + detail::fmt(pt.simulationDateTime) + "'";
            msg += " simulatedDateTime='" + detail::fmt(pt.simulatedDateTime) + "'";
            msg += " referenceDateTime='" + detail::fmt(pt.referenceDateTime) + "'";
            msg += " windowStart='" + detail::fmt(pt.windowStart) + "'";
            msg += " windowEnd='" + detail::fmt(pt.windowEnd) + "'";
            msg += " statisticalWindowCount='" + std::to_string(pt.statisticalWindowCount) + "'";
            msg += " statisticalWindows=" + detail::fmt(pt.statisticalWindows, pt.statisticalWindowCount);
            msg += " timeIncrementInSeconds='" +
                   (pt.timeIncrementInSeconds.has_value() ? std::to_string(pt.timeIncrementInSeconds.value())
                                                          : std::string("missing")) +
                   "'";
            return msg;
        }());

        return pt;
    }
    catch (...) {

        // §11: nested rethrow with context.
        std::throw_with_nested(Mars2GribDeductionException("Unable to resolve ProductTime", Here()));
    }

    // Remove compiler warning
    mars2gribUnreachable();
}

///
/// @brief Number of statistical time ranges encoded in a `ProductTime`.
///
/// This is a thin accessor: it returns `pt.statisticalWindowCount` cast to
/// `long`, which is the GRIB-side `numberOfTimeRanges` value (PDT 4.8 / 4.11).
///
/// Provided as a free function so that consumers do not need to access
/// the `ProductTime` field directly nor reach into `std::size_t` arithmetic.
///
/// @param[in] pt Resolved `ProductTime`.
/// @return Number of statistical loops (`0` for instant products).
///
inline long numberOfTimeRanges(const detail::ProductTime& pt) {
    return static_cast<long>(pt.statisticalWindowCount);
}

}  // namespace metkit::mars2grib::backend::deductions

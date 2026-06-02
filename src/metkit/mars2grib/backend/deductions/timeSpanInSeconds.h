/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <string>

#include "metkit/mars2grib/utils/generalUtils.h"

// Exceptions
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the time span from the MARS dictionary and convert it to seconds.
///
/// The value stored under the @c timespan key may be either:
/// - a @c long (hours), or
/// - a @c std::string with an optional unit suffix.
///
/// String conversion rules:
/// - @c "Xm"  (e.g. @c "10m") -> X * 60 seconds
/// - @c "Xh"  (e.g. @c "6h")  -> X * 3600 seconds
/// - @c "fs" / @c "from-start" / @c "fromstart" -> @c step * 3600 seconds
///   (@c step must be present in @p mars)
/// - Plain numeric string @c "N" -> N * 3600 seconds (treated as hours)
///
/// A @c long value N is always treated as hours: result = N * 3600.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TimeSpanInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        // --- Numeric path: long N interpreted as hours ---
        if (auto v = get_opt<long>(mars, "timespan")) {
            const long seconds = *v * 3600L;
            MARS2GRIB_LOG_RESOLVE([&]() {
                return "timeSpan: " + std::to_string(*v) + "h = " + std::to_string(seconds) + " [seconds]";
            }());
            return seconds;
        }

        // --- String path: parse unit suffix ---
        const std::string raw = get_or_throw<std::string>(mars, "timespan");

        // "fs" / "from-start" / "fromstart" -> step hours
        if (raw == "fs" || raw == "from-start" || raw == "fromstart") {
            const long step    = get_or_throw<long>(mars, "step");
            const long seconds = step * 3600L;
            MARS2GRIB_LOG_RESOLVE([&]() {
                return "timeSpan: fs resolved from step=" + std::to_string(step) + " -> " + std::to_string(seconds) + " [seconds]";
            }());
            return seconds;
        }

        // "Xm" suffix — minutes
        if (raw.size() > 1 && raw.back() == 'm') {
            const long minutes = std::stol(raw.substr(0, raw.size() - 1));
            const long seconds = minutes * 60L;
            MARS2GRIB_LOG_RESOLVE([&]() {
                return "timeSpan: " + raw + " = " + std::to_string(seconds) + " [seconds]";
            }());
            return seconds;
        }

        // "Xh" suffix — hours
        if (raw.size() > 1 && raw.back() == 'h') {
            const long hours   = std::stol(raw.substr(0, raw.size() - 1));
            const long seconds = hours * 3600L;
            MARS2GRIB_LOG_RESOLVE([&]() {
                return "timeSpan: " + raw + " = " + std::to_string(seconds) + " [seconds]";
            }());
            return seconds;
        }

        // Plain numeric string — treated as hours
        const long hours   = std::stol(raw);
        const long seconds = hours * 3600L;
        MARS2GRIB_LOG_RESOLVE([&]() {
            return "timeSpan: " + raw + "h = " + std::to_string(seconds) + " [seconds]";
        }());
        return seconds;
    }
    catch (const Mars2GribDeductionException&) {
        throw;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Unable to resolve `timespan` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

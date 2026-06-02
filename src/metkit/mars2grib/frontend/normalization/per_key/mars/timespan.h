/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include <string>

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Sanitize the MARS key: timespan.
///
/// The sanitizer is intentionally thin: it only filters out the
/// "instantaneous" sentinels and passes the raw value through to scratch
/// unchanged.  All unit conversion (hours -> seconds, Xm, Xh, fs) is
/// performed by the backend @c timeSpanInSeconds deduction.
///
/// Pass-through rules:
/// - Native @c long N -> stored as @c long N (interpreted as hours by backend)
/// - Any other string -> stored as @c std::string verbatim for backend parsing
/// - String "none" / "inst" / "instantaneous" -> no-op (key stays absent)
/// - Absent key -> no-op
///
template <typename MarsDict_t>
void sanitise_timespan_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& /*language*/) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    // String path: filter sentinels, pass everything else through verbatim
    if (auto s = get_opt<std::string>(in, "timespan")) {
        if (*s == "none" || *s == "inst" || *s == "instantaneous") {
            return;
        }
        set_or_throw<std::string>(out, "timespan", *s);
        return;
    }

    // Numeric path: store hours as-is; backend multiplies by 3600
    if (auto v = get_opt<long>(in, "timespan")) {
        set_or_throw<long>(out, "timespan", *v);
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

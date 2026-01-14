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

namespace metkit::mars2grib::utils {

template <typename OptDict, typename OutDict>
inline bool checksEnabled(const OptDict& opt) {

    using metkit::mars2grib::utils::dict_traits::dict_supports_checks_v;
    using metkit::mars2grib::utils::dict_traits::get_opt;

    if constexpr (!dict_supports_checks_v<OutDict>) {
        return false;
    }
    else {
        return get_opt<bool>(opt, "applyChecks").value_or(true);
    }


    // Remove compiler warning
    __builtin_unreachable();
}

template <typename OptDict>
inline bool overrideEnabled(const OptDict& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;

    return get_opt<bool>(opt, "enableOverride").value_or(false);
}

}  // namespace metkit::mars2grib::utils
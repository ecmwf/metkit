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

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::utils {

template <typename OutDict, class OptDict_t>
inline bool checksEnabled(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::dict_supports_checks_v;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if constexpr (!dict_supports_checks_v<OutDict>) {
        return false;
    }

    if (has(opt, "applyChecks")) {
        return get_or_throw<bool>(opt, "applyChecks");
    }
    else {
        return true;
    }
}

template <class OptDict_t>
inline bool skipSection3(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(opt, "skipSection3")) {
        return get_or_throw<bool>(opt, "skipSection3");
    }
    else {
        return false;
    }
}

template <class OptDict_t>
inline bool overrideEnabled(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(opt, "enableOverride")) {
        return get_or_throw<bool>(opt, "enableOverride");
    }
    else {
        return false;
    }
}

template <class OptDict_t>
inline bool bitsPerValueCompressionEnabled(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(opt, "enableBitsPerValueCompression")) {
        return get_or_throw<bool>(opt, "enableBitsPerValueCompression");
    }
    else {
        return false;
    }
}

template <class OptDict_t>
inline bool normalizeMarsEnabled(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(opt, "normalizeMars")) {
        return get_or_throw<bool>(opt, "normalizeMars");
    }
    else {
        return false;
    }
}

template <class OptDict_t>
inline bool normalizeMiscEnabled(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(opt, "normalizeMisc")) {
        return get_or_throw<bool>(opt, "normalizeMisc");
    }
    else {
        return false;
    }
}

template <class OptDict_t>
inline bool fixMarsGridEnabled(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(opt, "fixMarsGrid")) {
        return get_or_throw<bool>(opt, "fixMarsGrid");
    }
    else {
        return false;
    }
}

}  // namespace metkit::mars2grib::utils

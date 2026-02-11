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

template <typename OutDict>
inline bool checksEnabled(const Options& opt) {
    using metkit::mars2grib::utils::dict_traits::dict_supports_checks_v;

    if constexpr (!dict_supports_checks_v<OutDict>) {
        return false;
    }
    return opt.applyChecks;
}

inline bool overrideEnabled(const Options& opt) {
    return opt.enableOverride;
}

inline bool bitsPerValueCompressionEnabled(const Options& opt) {
    return opt.enableBitsPerValueCompression;
}

inline bool normalizeMarsEnabled(const Options& opt) {
    return opt.normalizeMars;
}

inline bool normalizeMiscEnabled(const Options& opt) {
    return opt.normalizeMisc;
}

inline bool fixMarsGridEnabled(const Options& opt) {
    return opt.fixMarsGrid;
}

}  // namespace metkit::mars2grib::utils

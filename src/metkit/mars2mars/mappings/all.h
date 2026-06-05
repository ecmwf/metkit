/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars {

using metkit::mars2mars::utils::dict_traits::get_or_throw;
using metkit::mars2mars::utils::dict_traits::set_or_throw;

using metkit::mars2mars::util::param_matcher::matchAny;
using metkit::mars2mars::util::param_matcher::range;

template <class OutDict_t>
inline void setParamLevel(OutDict_t& out, long param, const std::string& levtype, long levelist) {
    set_or_throw<long>(out, "param", param);
    set_or_throw<std::string>(out, "levtype", levtype);
    set_or_throw<long>(out, "levelist", levelist);
}

template <class InDict_t, class OutDict_t>
inline void convertSFC2SOL(const InDict_t& in, OutDict_t& out) {
    const auto param = get_or_throw<long>(in, "param");
    switch (param) {
        // (35, 36, 37, 39) -> 262024
        case 35:
            return setParamLevel(out, 262024, "sol", 1);
        case 36:
            return setParamLevel(out, 262024, "sol", 2);
        case 37:
            return setParamLevel(out, 262024, "sol", 3);
        case 38:
            return setParamLevel(out, 262024, "sol", 4);

        // (39, 40, 41, 42) -> 260199
        case 39:
            return setParamLevel(out, 260199, "sol", 1);
        case 40:
            return setParamLevel(out, 260199, "sol", 2);
        case 41:
            return setParamLevel(out, 260199, "sol", 3);
        case 42:
            return setParamLevel(out, 260199, "sol", 4);

        // (139, 170, 183, 236) -> 260360
        case 139:
            return setParamLevel(out, 260360, "sol", 1);
        case 170:
            return setParamLevel(out, 260360, "sol", 2);
        case 183:
            return setParamLevel(out, 260360, "sol", 3);
        case 236:
            return setParamLevel(out, 260360, "sol", 4);
    }
}

template <class InDict_t, class OutDict_t>
OutDict_t convertAll(const InDict_t& in) {
    using metkit::mars2mars::utils::dict_traits::clone_or_throw;
    std::unique_ptr<OutDict_t> out = clone_or_throw(in);

    convertSFC2SOL(in, *out);

    return *out;
}

}

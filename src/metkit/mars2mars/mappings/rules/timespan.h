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

/// @file timespan.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/common.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {

/// @brief Assign `param`, and `timespan` together.
template <class OutDict_t>
inline void setParamTimespan(OutDict_t& out, long param, const std::string& timespan) {

    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        set_or_throw<long>(out, "param", param);
        set_or_throw<std::string>(out, "timespan", timespan);
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to set param and/or timespan in output dictionary", Here()));
    }
}


/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t>
inline void fixTimespan(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        const auto param = get_or_throw<long>(in, "param");

        switch (param) {
            // Max 2t
            case 121:
                return setParamTimespan(out, 237167, "6h");
            case 228026:
                return setParamTimespan(out, 237167, "3h");
            case 201:
                return set_or_throw<long>(out, "param", 237167);

            // Max 6h CAPE
            case 228035:
                return setParamTimespan(out, 237117, "6h");

            // Max 6h CAPES
            case 228036:
                return setParamTimespan(out, 237321, "6h");

            // Max precipitation
            case 228222:
                return setParamTimespan(out, 237055, "3h");
            case 228224:
                return setParamTimespan(out, 237055, "6h");
            case 228226:
                return set_or_throw<long>(out, "param", 237055);

            // Max wind gust
            case 123:
                return setParamTimespan(out, 237318, "6h");
            case 228028:
                return setParamTimespan(out, 237318, "3h");
            case 49:
                return set_or_throw<long>(out, "param", 237318);

            // Mean flash density ?
            case 228051:
                return setParamTimespan(out, 235326, "1h");
            case 228057:
                return setParamTimespan(out, 235326, "3h");
            case 228058:
                return setParamTimespan(out, 235326, "6h");

            // Min 2t
            case 122:
                return setParamTimespan(out, 238167, "6h");
            case 228027:
                return setParamTimespan(out, 238167, "3h");
            case 202:
                return set_or_throw<long>(out, "param", 238167);

            // Min precipitation
            case 228223:
                return setParamTimespan(out, 238055, "3h");
            case 228225:
                return setParamTimespan(out, 238055, "6h");
            case 228227:
                return set_or_throw<long>(out, "param", 238055);

            // Most frequent precipitation type
            case 260320:
                return setParamTimespan(out, 260683, "1h");
            case 260321:
                return setParamTimespan(out, 260683, "3h");
            case 260339:
                return setParamTimespan(out, 260683, "6h");

            // Most severe precipitation type
            case 260318:
                return setParamTimespan(out, 260682, "1h");
            case 260319:
                return setParamTimespan(out, 260682, "3h");
            case 260338:
                return setParamTimespan(out, 260682, "6h");

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convert input dictionary in fixTimespan", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl

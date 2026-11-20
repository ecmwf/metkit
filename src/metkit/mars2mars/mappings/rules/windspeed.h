// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file windspeed.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/common.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void fixWindspeed(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc, const OptDict_t& opts) {

    using metkit::mars2mars::utils::dict_traits::get_opt;
    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        (void)opts;
        const auto levtype = get_opt<std::string>(in, "levtype");
        if (levtype && *levtype == "sfc") {
            const auto param = get_or_throw<long>(in, "param");
            switch (param) {
                case 228246:  // 100u
                    return detail::setParamLevel(out, 131, "hl", 100);
                case 228239:  // 200u
                    return detail::setParamLevel(out, 131, "hl", 200);

                case 228247:  // 100v
                    return detail::setParamLevel(out, 132, "hl", 100);
                case 228240:  // 200v
                    return detail::setParamLevel(out, 132, "hl", 200);

                case 228249:  // 100si
                    return detail::setParamLevel(out, 10, "hl", 100);
                case 228241:  // 200si
                    return detail::setParamLevel(out, 10, "hl", 200);

                default:
                    break;
            }
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convert input dictionary in fixWindspeed", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl

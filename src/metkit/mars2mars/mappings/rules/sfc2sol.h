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

/// @file all.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {

namespace detail {

/// @brief Assign `param`, `levtype`, and `levelist` together.
template <class OutDict_t>
inline void setParamLevel(OutDict_t& out, long param, const std::string& levtype, long levelist) {

    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;

    try {
        set_or_throw<long>(out, "param", param);
        set_or_throw<std::string>(out, "levtype", levtype);
        set_or_throw<long>(out, "levelist", levelist);
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to setParamLevel to input dictionaries", Here()));
    }
}

} // namespace detail

/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t>
inline void convertSFC2SOL(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;
    using metkit::mars2mars::utils::dict_traits::get_or_throw;

    try {
        const auto param = get_or_throw<long>(in, "param");

        switch (param) {
            // (35, 36, 37, 39) -> 262024
            case 35:
                return detail::setParamLevel(out, 262024, "sol", 1);
            case 36:
                return detail::setParamLevel(out, 262024, "sol", 2);
            case 37:
                return detail::setParamLevel(out, 262024, "sol", 3);
            case 38:
                return detail::setParamLevel(out, 262024, "sol", 4);

            // (39, 40, 41, 42) -> 260199
            case 39:
                return detail::setParamLevel(out, 260199, "sol", 1);
            case 40:
                return detail::setParamLevel(out, 260199, "sol", 2);
            case 41:
                return detail::setParamLevel(out, 260199, "sol", 3);
            case 42:
                return detail::setParamLevel(out, 260199, "sol", 4);

            // (139, 170, 183, 236) -> 260360
            case 139:
                return detail::setParamLevel(out, 260360, "sol", 1);
            case 170:
                return detail::setParamLevel(out, 260360, "sol", 2);
            case 183:
                return detail::setParamLevel(out, 260360, "sol", 3);
            case 236:
                return detail::setParamLevel(out, 260360, "sol", 4);
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convertSFC2SOL input dictionaries", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl

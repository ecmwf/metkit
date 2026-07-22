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

/// @file incrementalType4i.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Convert incremental fields
template <class InDict_t, class OutDict_t>
inline void convertIncremental(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        const auto param = get_or_throw<long>(in, "param");
        switch (param) {
            case 200130:
                set_or_throw<long>(out, "param", 130);
                break;

            case 200133:
                set_or_throw<long>(out, "param", 133);
                break;

            case 200138:
                set_or_throw<long>(out, "param", 138);
                break;

            case 200152:
                set_or_throw<long>(out, "param", 152);
                break;

            case 200155:
                set_or_throw<long>(out, "param", 155);
                break;

            case 200203:
                set_or_throw<long>(out, "param", 203);
                break;

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to convert input dictionary in convertIncremental", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl

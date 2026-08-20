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

/// @file removeNumber.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Remove MARS key number if no longer used
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void removeNumber(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc, const OptDict_t& opts) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        (void)opts;
        if (get_or_throw<std::string>(in, "type") == "me") {
            setMissing_or_throw(out, "number");
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convert input dictionary in removeNumber", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl

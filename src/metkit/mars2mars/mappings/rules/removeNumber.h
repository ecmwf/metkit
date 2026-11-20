// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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

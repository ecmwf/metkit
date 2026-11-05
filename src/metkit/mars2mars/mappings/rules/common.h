// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file common.h
/// @brief Shared utilities to make conversion rules easier to implement.
/// @todo: utils/paramMatcher.h needs to be merged into this file.
#pragma once

#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl::detail {

/// @brief Assign `param`, `levtype`, and `levelist` together.
template <class OutDict_t>
inline void setParamLevel(OutDict_t& out, long param, const std::string& levtype, long levelist) {

    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

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

}  // namespace metkit::mars2mars::rules::impl::detail

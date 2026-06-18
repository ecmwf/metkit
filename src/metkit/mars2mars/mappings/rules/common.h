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

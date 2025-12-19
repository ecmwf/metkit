#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
std::string marsType(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.type
        auto marsTypeVal = get_or_throw<std::string>(mars, "type");

        // TODO MIVAL: Validate

        return marsTypeVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `type` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

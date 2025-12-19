#pragma once

#include <algorithm>
#include <exception>
#include <string>
#include <string_view>
#include <vector>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
std::vector<double> pvArray(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.number
        std::vector<double> marsNumberVal = get_or_throw<std::vector<double>>(par, "pv");

        // TODO MIVAL: Add map of pv arrays, if pv is not present in parametrization
        // The search for a "pvSize" and try to lookup a custom pv array from the size.

        // TODO: MIVAL: decide if we want to fallback to a default size

        // TODO MIVAL: Validate

        return marsNumberVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `pvArray` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

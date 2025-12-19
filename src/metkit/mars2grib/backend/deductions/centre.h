#pragma once

#include <exception>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
std::string centre(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Lookup origin from the mars dictionary
        auto origin = get_or_throw<std::string>(mars, "origin");


        // Return validated origin
        return origin;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `origin` as string from Mars dictionary", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

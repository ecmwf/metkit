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
long marsFrequency_or_throw(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.freq
        auto marsFreqVal = get_or_throw<long>(mars, "frequency");

        // TODO MIVAL: Validate

        return marsFreqVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `freq` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

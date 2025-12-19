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

namespace metkit::mars2grib::backend::checks {

template <class OptDict_t, class OutDict_t>
void hasLocalUseSection_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (bool applyChecks = get_opt<bool>(opt, "applyChecks").value_or(false); applyChecks) {
            // Get the mars.date and mars.time
            long hasLocalUseSection = get_or_throw<long>(out, "LocalUsePresent");

            if (hasLocalUseSection == 0) {
                throw Mars2GribDeductionException("LocalUseSection not present in the sample", Here());
            }
        }

        // Exit point with success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to check presence of Local Use Section from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::checks

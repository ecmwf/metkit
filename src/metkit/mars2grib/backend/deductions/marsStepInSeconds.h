#pragma once

#include <algorithm>
#include <array>
#include <exception>
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

// Utils
#include "metkit/mars2grib/utils/timeUtils.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
long marsStepInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;
    using metkit::mars2grib::utils::time::toSeconds_or_throw;

    try {

        // Get the mars.step
        long marsStepVal = get_or_throw<long>(mars, "step");

        // Convert to seconds
        // long marsStepInSecondsVal = toSeconds_or_throw( marsStepVal );
        long marsStepInSecondsVal = marsStepVal * 3600;  // TODO: Multiply by timestep instead

        // TODO MIVAL: Validate

        return marsStepInSecondsVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `step` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

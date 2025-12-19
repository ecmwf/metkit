#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>


#include <eckit/types/DateTime.h>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

// Utils
#include "metkit/mars2grib/utils/timeUtils.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
eckit::DateTime forecastDateTime_or_throw(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;
    using metkit::mars2grib::utils::time::toSeconds_or_throw;

    try {

        // TODO MIVAL. A lot of assumptions here!!!!!

        // Get the mars.date, mars.time, mars.step
        long marsDate = get_or_throw<long>(mars, "date");
        long marsTime = get_or_throw<long>(mars, "time");
        long marsStep = get_or_throw<long>(mars, "step");

        // Convert step in seconds
        // long marsStepInSecondsVal = toSeconds_or_throw( marsStep );
        long marsStepInSecondsVal = marsStep * 3600;  // TODO: Multiply by timestep

        // Compute forecast time in seconds since reference time

        long YYYY = marsDate / 10000;
        long MM   = (marsDate / 100) % 100;
        long DD   = marsDate % 100;
        long hh   = marsTime / 10000;
        long mm   = (marsTime / 100) % 100;
        long ss   = marsTime % 100;

        eckit::DateTime referenceTime{eckit::Date(YYYY, MM, DD), eckit::Time(hh, mm, ss)};

        return referenceTime + static_cast<eckit::Second>(marsStepInSecondsVal);
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to compute forecast time", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
eckit::DateTime referenceDateTime(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // TODO MIVAL: get as string and parse/normalize with metkit utilities

        // Get the mars.date and mars.time
        auto marsDate = get_or_throw<long>(mars, "date");
        auto marsTime = get_or_throw<long>(mars, "time");

        long YYYY = marsDate / 10000;
        long MM   = (marsDate / 100) % 100;
        long DD   = marsDate % 100;
        long hh   = marsTime / 10000;
        long mm   = (marsTime / 100) % 100;
        long ss   = marsTime % 100;

        return eckit::DateTime(eckit::Date(YYYY, MM, DD), eckit::Time(hh, mm, ss));
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `date` and `time` from Mars dictionary to deduce `dateTime`", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
#include "metkit/mars2grib/backend/deductions/detail/timeUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
long numberOfTimeRanges(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::backend::deductions::detail::countBlocks;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.levelist
        bool hasTimespan = has(mars, "timespan");
        bool hasStatType = has(mars, "stattype");

        // Error if timespan is missing
        if (!hasTimespan) {
            throw Mars2GribDeductionException("`timespan` is required to compute number of time ranges", Here());
        }
        // Handle trivial case
        if (hasTimespan || !hasStatType) {
            return 1;
        }

        if (hasStatType) {

            // Get the stattype
            std::string statTypeVal = get_or_throw<std::string>(mars, "stattype");

            // TODO MIVAL: Validate stattype

            // Count number of blocks in stattype
            long numberOfBlocks = static_cast<long>(countBlocks(statTypeVal));

            // Number of time ranges = number of blocks + 1
            return numberOfBlocks + 1;
        }

        // Remove compiler warning
        __builtin_unreachable();
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `levelist` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

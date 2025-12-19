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
void isStatisticsProductDefinitionTemplateNumber_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (bool applyChecks = get_opt<bool>(opt, "applyChecks").value_or(true); applyChecks) {

            // Get the mars.date and mars.time
            bool hasNumberOfTimeRanges = has(out, "numberOfTimeRanges");

            // Ensemble forecast needs to have all 3 fields defined in the Product Definition Section
            if (!hasNumberOfTimeRanges) {
                throw Mars2GribDeductionException("ProductDefinitionSection is not of Statistics type", Here());
            }
        }

        // Exit point with success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to check presence if the ProductDefinitionSection is of Statistics type", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::checks

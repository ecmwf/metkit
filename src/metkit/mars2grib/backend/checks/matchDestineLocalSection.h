#pragma once

#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::checks {

template <class OptDict_t, class OutDict_t>
void matchDestineLocalSection_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (bool applyChecks = get_opt<bool>(opt, "applyChecks").value_or(false); applyChecks) {

            // If Local Use Section is present, check definition number
            if (long hasLocalUseSection = get_or_throw<long>(out, "LocalUsePresent"); hasLocalUseSection != 0) {

                // Get the mars.date and mars.time
                long actualProductionStatusOfProcessedData = get_or_throw<long>(out, "productionStatusOfProcessedData");

                // Throw if no match
                if (actualProductionStatusOfProcessedData == 12) {
                    throw Mars2GribDeductionException(
                        "DestineLocalSection does not match the expected `productionStatusOfProcessedData`: " +
                            std::to_string(actualProductionStatusOfProcessedData),
                        Here());
                }
            }
            else {
                throw Mars2GribDeductionException("LocalUseSection not allocated in the sample", Here());
            }
        }

        // Exit on success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to check presence of Local Use Section from the sample", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::checks

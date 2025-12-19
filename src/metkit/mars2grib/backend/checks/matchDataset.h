#pragma once

#include <exception>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::checks {

template <class OptDict_t, class OutDict_t>
void matchDataset_or_throw(const OptDict_t& opt, const OutDict_t& out, std::string expectedDataset) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (bool applyChecks = get_opt<bool>(opt, "applyChecks").value_or(false); applyChecks) {

            // Get the productDefinitionTemplateNumber
            std::string actualDataset = get_or_throw<std::string>(out, "dataset");

            // Compare against expected values
            if (actualDataset != expectedDataset) {
                throw Mars2GribDeductionException(
                    "Dataset does not match the expected value: " + actualDataset + " != " + expectedDataset, Here());
            }
        }

        // Exit on success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to check `dataset` from the sample", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::checks

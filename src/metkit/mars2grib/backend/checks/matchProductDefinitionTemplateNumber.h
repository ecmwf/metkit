#pragma once

#include <exception>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::checks {

template <class OptDict_t, class OutDict_t>
void matchProductDefinitionTemplateNumber_or_throw(const OptDict_t& opt, const OutDict_t& out,
                                                   std::vector<long> expectedProductDefinitionTemplateNumber) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (bool applyChecks = get_opt<bool>(opt, "applyChecks").value_or(false); applyChecks) {

            // Get the productDefinitionTemplateNumber
            long actualProductDefinitionTemplateNumber = get_or_throw<long>(out, "productDefinitionTemplateNumber");

            // Compare against expected values
            bool match = false;
            for (const auto& expectedDefNum : expectedProductDefinitionTemplateNumber) {
                if (actualProductDefinitionTemplateNumber == expectedDefNum) {
                    match = true;
                    break;
                }
            }

            // Throw if no match
            if (!match) {
                throw Mars2GribDeductionException(
                    "ProductDefinitionTemplateNumber does not match any of the expected values: " +
                        std::to_string(actualProductDefinitionTemplateNumber),
                    Here());
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

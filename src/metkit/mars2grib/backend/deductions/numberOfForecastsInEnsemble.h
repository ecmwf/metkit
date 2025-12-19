#pragma once

#include <cstdint>
#include <optional>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {


template <class MarsDict_t, class ParDict_t>
long numberOfForecastsInEnsemble(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // The only way to infer this is from parametrization
        long numberOfForecastsInEnsemble = get_or_throw<long>(par, "numberOfForecastsInEnsemble");
        long perturbationNumber          = get_or_throw<long>(mars, "number");

        // Basic validation
        if (perturbationNumber < 0 || perturbationNumber > numberOfForecastsInEnsemble) {
            throw Mars2GribDeductionException("`perturbationNumber` must be in range [0, numberOfForecastsInEnsemble)",
                                              Here());
        }

        return numberOfForecastsInEnsemble;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `numberOfForecastsInEnsemble` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

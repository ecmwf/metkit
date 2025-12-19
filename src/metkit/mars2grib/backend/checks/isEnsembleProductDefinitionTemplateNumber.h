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
void isEnsembleProductDefinitionTemplateNumber_or_throw(const OptDict_t& opt, const OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (bool applyChecks = get_opt<bool>(opt, "applyChecks").value_or(false); applyChecks) {

            // Get the mars.date and mars.time
            bool has_TypeOfEnsembleForecast      = has(out, "typeOfEnsembleForecast");
            bool has_PerturbationNumber          = has(out, "perturbationNumber");
            bool has_NumberOfForecastsInEnsemble = has(out, "numberOfForecastsInEnsemble");

            // Ensemble forecast needs to have all 3 fields defined in the Product Definition Section
            if (!(has_TypeOfEnsembleForecast && has_PerturbationNumber && has_NumberOfForecastsInEnsemble)) {
                throw Mars2GribDeductionException("ProductDefinitionSection is not of Ensemble type", Here());
            }
        }

        // Exit point with success
        return;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to check presence if the ProductDefinitionSection is of Ensemble type", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::checks

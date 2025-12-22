/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `numberOfForecastsInEnsemble` key.
 *
 * This deduction determines the value of the GRIB
 * `numberOfForecastsInEnsemble` key, which specifies the total number
 * of ensemble members in the forecast system.
 *
 * @details
 * The value is **not inferable** from the MARS request alone and must
 * therefore be provided explicitly via the parametrization dictionary
 * (`par`).
 *
 * The MARS key `number` (perturbation number) is used only for
 * **consistency validation** and does not influence the returned value.
 *
 * ### Validation performed
 * - `numberOfForecastsInEnsemble` must be present in the parameter
 *   dictionary.
 * - `mars::number` (perturbation number) must satisfy:
 *
 *   ```
 *   0 ≤ perturbationNumber ≤ numberOfForecastsInEnsemble
 *   ```
 *
 * Any violation results in a deduction error.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `numberOfForecastsInEnsemble`.
 * No defaulting or implicit inference is performed.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary; must contain the key `number`
 * @param[in] par  Parameter dictionary; must contain
 *                 `numberOfForecastsInEnsemble`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The total number of forecasts in the ensemble.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - `numberOfForecastsInEnsemble` is missing from the parameter dictionary
 *         - `mars::number` is missing
 *         - the perturbation number is outside the valid range
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction is fully deterministic.
 * - No reliance on pre-existing GRIB header state is allowed.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: policy][prio: medium]
 * - Define whether `mars::number == 0` always corresponds to the
 *   control member or whether this must be explicitly encoded.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_NumberOfForecastsInEnsemble_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

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

        // Logging of the par::lengthOfTimeWindow
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "numberOfForecastsInEnsemble: looked up from Par dictionary with value: ";
            logMsg += std::to_string(numberOfForecastsInEnsemble);
            return logMsg;
        }());

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

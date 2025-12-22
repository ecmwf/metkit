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
 * @brief Resolve the experiment identifier from the MARS dictionary.
 *
 * This deduction retrieves the value associated with the key `experiment`
 * from the MARS dictionary (`mars`). The value is expected to be a
 * string and is treated as mandatory.
 *
 * The resolved value typically identifies the experiment or experiment
 * family under which the data was produced (e.g. control runs,
 * sensitivity experiments, or development/test configurations).
 * The semantics and allowed values of the experiment identifier are
 * defined by upstream MARS conventions and are not interpreted by
 * this deduction.
 *
 * The resolved value is logged for diagnostic and traceability
 * purposes.
 *
 * If the key is missing or the value cannot be converted to the
 * expected type, a domain-specific exception is thrown. Any underlying
 * error is propagated using nested exceptions to preserve full
 * diagnostic context.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `experiment`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the experiment identifier is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The experiment identifier resolved from the MARS dictionary,
 *   returned as a `std::string`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `experiment` is not present in the MARS dictionary,
 *   - the associated value cannot be converted to `std::string`,
 *   - any unexpected error occurs during dictionary access.
 *
 * @note
 *   This deduction assumes that the experiment identifier is explicitly
 *   provided by the MARS dictionary and does not attempt any inference,
 *   defaulting, or validation of the returned value.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested
 *   exception propagation to ensure that error provenance is
 *   preserved across API boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Experiment_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.experiment
        std::string marsExperimentVal = get_or_throw<std::string>(mars, "experiment");

        // Logging of the experiment
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "experiment: deduced from mars dictionary with value: " + marsExperimentVal;
            return logMsg;
        }());

        return marsExperimentVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `experiment` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
 * @brief Resolve the GRIB `scaledValueOfCentralWaveNumber` key.
 *
 * This deduction resolves the value of the GRIB
 * `scaledValueOfCentralWaveNumber` key, which is used in satellite products
 * representations together with `scaleFactorOfCentralWaveNumber`
 * to encode the central wave number.
 *
 * The value is obtained **exclusively** from the parameter dictionary
 * (`par`). No deduction from MARS metadata or GRIB header state
 * is attempted.
 *
 * @important
 * This function assumes that `scaledValueOfCentralWaveNumber` is
 * explicitly provided via parametrization.
 * There is currently no defaulting or inference logic.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; must contain the key
 *                 `scaledValueOfCentralWaveNumber`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved scaled value of the central wave number
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `scaledValueOfCentralWaveNumber` is missing from
 *           the parameter dictionary
 *         - the value cannot be retrieved or converted to `long`
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - Numerical consistency between `scaledValueOfCentralWaveNumber`
 *   and `scaleFactorOfCentralWaveNumber` is not validated here.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: validation][prio: low]
 * - Introduce cross-validation between
 *   `scaledValueOfCentralWaveNumber` and
 *   `scaleFactorOfCentralWaveNumber` once formal constraints are
 *   defined by the GRIB specification or ECMWF policy.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ScaledValueOfCentralWaveNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                     const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the par.scaledvalueofcentralwavenumber
        auto scaledValueOfCentralWaveNumberVal = get_or_throw<long>(par, "scaledValueOfCentralWaveNumber");

        // Logging of the par::lengthOfTimeWindow
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "scaledValueOfCentralWaveNumber: looked up from Par dictionary with value: ";
            logMsg += std::to_string(scaledValueOfCentralWaveNumberVal);
            return logMsg;
        }());

        // Return value
        return scaledValueOfCentralWaveNumberVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `scaledValueOfCentralWaveNumber` from PAr dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

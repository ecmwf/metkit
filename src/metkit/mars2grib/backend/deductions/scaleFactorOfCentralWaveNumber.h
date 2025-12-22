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

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `scaleFactorOfCentralWaveNumber` key.
 *
 * This deduction resolves the value of the GRIB
 * `scaleFactorOfCentralWaveNumber` key, which is used in satellite products
 * representations to scale the central wave number.
 *
 * The value is obtained **exclusively** from the parameter dictionary
 * (`par`). No automatic deduction from MARS metadata is attempted.
 *
 * @important
 * This function assumes that `scaleFactorOfCentralWaveNumber` is
 * explicitly provided via parametrization.
 * There is currently no defaulting or inference logic.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; must contain the key
 *                 `scaleFactorOfCentralWaveNumber`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved scale factor of the central wave number
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `scaleFactorOfCentralWaveNumber` is missing from
 *           the parameter dictionary
 *         - the value cannot be retrieved or converted to `long`
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - No validation of the numerical range is currently performed.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: validation][prio: low]
 * - Introduce validation of the scale factor range once formal
 *   constraints are defined in the GRIB specification or ECMWF policy.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ScaleFactorOfCentralWaveNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                     const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the par.scalefactorofcentralwavenumber
        auto scaleFactorOfCentralWaveNumberVal = get_or_throw<long>(par, "scaleFactorOfCentralWaveNumber");

        // Logging of the par::lengthOfTimeWindow
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "scaleFactorOfCentralWaveNumber: looked up from Par dictionary with value: ";
            logMsg += std::to_string(scaleFactorOfCentralWaveNumberVal);
            return logMsg;
        }());

        // Return value
        return scaleFactorOfCentralWaveNumberVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `scaleFactorOfCentralWaveNumber` from PAr dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

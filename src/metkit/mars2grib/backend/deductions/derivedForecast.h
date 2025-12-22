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

// Tables
#include "metkit/mars2grib/backend/tables/derivedForecast.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `derivedForecast` key.
 *
 * This deduction determines the value of the GRIB
 * `derivedForecast` key (GRIB2 Code Table 4.7) used to describe
 * the nature of a derived Forecast.
 *
 * Resolution follows a strict precedence order:
 *
 * 1. **User override (Par dictionary)**
 *    If the key `derivedForecast` is present in the parameter
 *    dictionary (`par`), its numeric value is taken as authoritative.
 *    The value is validated by converting it to the corresponding
 *    `derivedForecast` enumeration via the GRIB table mapping.
 *
 * 2. **Automatic deduction (MARS dictionary)**
 *    If no override is provided, the value is deduced from the MARS
 *    key `type`
 *
 * Any unsupported `mars::type` value results in a deduction error.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `derivedForecast`.
 * All validation and mapping logic for this GRIB key must be implemented
 * here.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (currently unused)
 *
 * @param[in] mars MARS dictionary; must contain the key `type` when no
 *                 override is provided
 * @param[in] par  Parameter dictionary; may contain the key
 *                 `derivedForecast` as a numeric override
 * @param[in] opt  Options dictionary (currently unused)
 *
 * @return The resolved `derivedForecast` enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - an override value is provided but is not a valid GRIB code
 *         - the MARS key `type` is missing
 *         - the MARS `type` value is not mapped to a known ensemble
 *           forecast type
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does **not** rely on any pre-existing GRIB header state.
 * - The result is deterministic and reproducible.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: completeness][prio: medium]
 * - Extend automatic deduction logic when additional MARS `type`
 *   values are officially mapped to GRIB `derivedForecast`.
 * - Consider validating consistency with other ensemble-related keys
 *   (e.g. `numberOfForecastsInEnsemble`).
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::DerivedForecast resolve_DerivedForecast_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                         const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        // Get mars type from dictionary

        if (has(par, "derivedForecast")) {

            // User override from par dictionary
            long derivedForecastVal = get_or_throw<long>(par, "derivedForecast");

            // Get the enum value
            tables::DerivedForecast derivedForecast = tables::long2enum_DerivedForecast_or_throw(derivedForecastVal);

            // Logging of the par::derivedForecast
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "derivedForecast: override from par dictionary with value: ";
                logMsg += tables::enum2name_DerivedForecast_or_throw(derivedForecast);
                return logMsg;
            }());

            return derivedForecast;
        }
        else {

            // Deduce from mars.type
            std::string marsType = get_or_throw<std::string>(mars, "type");

            tables::DerivedForecast derivedForecast = tables::DerivedForecast::Missing;

            throw Mars2GribDeductionException("Not implemented", Here());

            // Logging of the par::derivedForecast
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "derivedForecast: deduced from mars.type with value: ";
                logMsg += tables::enum2name_DerivedForecast_or_throw(derivedForecast);
                return logMsg;
            }());

            return derivedForecast;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to deduce `derivedForecast` from Mars or Par dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

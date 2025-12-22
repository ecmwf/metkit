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
#include "metkit/mars2grib/backend/tables/typeOfEnsembleForecast.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `typeOfEnsembleForecast` key.
 *
 * This deduction determines the value of the GRIB
 * `typeOfEnsembleForecast` key (GRIB2 Code Table 4.6) used to describe
 * the nature of ensemble perturbations.
 *
 * Resolution follows a strict precedence order:
 *
 * 1. **User override (Par dictionary)**
 *    If the key `typeOfEnsembleForecast` is present in the parameter
 *    dictionary (`par`), its numeric value is taken as authoritative.
 *    The value is validated by converting it to the corresponding
 *    `TypeOfEnsembleForecast` enumeration via the GRIB table mapping.
 *
 * 2. **Automatic deduction (MARS dictionary)**
 *    If no override is provided, the value is deduced from the MARS
 *    key `type`:
 *    - `"cf"` → `Unperturbed`
 *    - `"pf"` → `Perturbed`
 *
 * Any unsupported `mars::type` value results in a deduction error.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `typeOfEnsembleForecast`.
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
 *                 `typeOfEnsembleForecast` as a numeric override
 * @param[in] opt  Options dictionary (currently unused)
 *
 * @return The resolved `TypeOfEnsembleForecast` enumeration value
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
 *   values are officially mapped to GRIB `typeOfEnsembleForecast`.
 * - Consider validating consistency with other ensemble-related keys
 *   (e.g. `numberOfForecastsInEnsemble`).
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::TypeOfEnsembleForecast resolve_TypeOfEnsembleForecast_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                       const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        // Get mars type from dictionary

        if (has(par, "typeOfEnsembleForecast")) {

            // User override from par dictionary
            long typeOfEnsembleForecastVal = get_or_throw<long>(par, "typeOfEnsembleForecast");

            // Get the enum value
            tables::TypeOfEnsembleForecast typeOfEnsembleForecast =
                tables::long2enum_TypeOfEnsembleForecast_or_throw(typeOfEnsembleForecastVal);

            // Logging of the par::typeOfEnsembleForecast
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "typeOfEnsembleForecast: override from par dictionary with value: ";
                logMsg += tables::enum2name_TypeOfEnsembleForecast_or_throw(typeOfEnsembleForecast);
                return logMsg;
            }());

            return typeOfEnsembleForecast;
        }
        else {

            // Deduce from mars.type
            std::string marsType = get_or_throw<std::string>(mars, "type");

            tables::TypeOfEnsembleForecast typeOfEnsembleForecast = tables::TypeOfEnsembleForecast::Missing;

            if (marsType == "cf") {
                typeOfEnsembleForecast = tables::TypeOfEnsembleForecast::Unperturbed;
            }
            else if (marsType == "pf") {
                typeOfEnsembleForecast = tables::TypeOfEnsembleForecast::Perturbed;
            }
            else {
                throw Mars2GribDeductionException(
                    "`type` value '" + marsType + "' is not mapped to any known `typeOfEnsembleForecast`", Here());
            }

            // Logging of the par::typeOfEnsembleForecast
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "typeOfEnsembleForecast: deduced from mars.type with value: ";
                logMsg += tables::enum2name_TypeOfEnsembleForecast_or_throw(typeOfEnsembleForecast);
                return logMsg;
            }());

            return typeOfEnsembleForecast;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to deduce `typeOfEnsembleForecast` from Mars or Par dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

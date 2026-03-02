/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file numberOfForecastsInEnsemble.h
/// @brief Deduction of the GRIB `numberOfForecastsInEnsemble` key.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **total number of forecasts in an ensemble**.
///
/// The value cannot be inferred from the MARS request alone and must be
/// provided explicitly via the parameter dictionary.
///
/// The MARS key `number` (perturbation number) is used exclusively for
/// consistency validation and does not affect the returned value.
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or invalid inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved from one or more input dictionaries
///
/// @section References
/// Concept:
/// - @ref ensembleEncoding.h
///
/// Related deductions:
/// - @ref perturbationNumber.h
/// - @ref typeOfEnsembleForecast.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `numberOfForecastsInEnsemble` key.
///
/// @section Deduction contract
/// - Reads: `par["numberOfForecastsInEnsemble"]`, `mars["number"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the total number of ensemble forecasts.
///
/// The value is taken verbatim from the parameter dictionary.
/// No inference, defaulting, or heuristic logic is applied.
///
/// The MARS perturbation number (`mars["number"]`) is optional and
/// used only for consistency validation.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Optionally provide `number`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary. Must provide `numberOfForecastsInEnsemble`.
/// Since no defaulting is implemented for this key yet, it must be explicitly provided.
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary providing the perturbation number.
///
/// @param[in] par
/// Parameter dictionary providing the ensemble size.
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The total number of forecasts in the ensemble.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If required keys are missing, if the perturbation number is outside
/// the valid range, or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction is fully deterministic and does not depend on
/// pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_NumberOfForecastsInEnsemble_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (has(par, "numberOfForecastsInEnsemble")) {
            // The only way to infer this is from parametrization
            const auto numberOfForecastsInEnsemble = get_or_throw<long>(par, "numberOfForecastsInEnsemble");
            const auto perturbationNumber          = get_opt<long>(mars, "number");

            // Basic validation
            if (perturbationNumber.has_value()) {
                if (*perturbationNumber < 0) {
                    std::string errMsg = "`perturbationNumber` (";
                    errMsg += std::to_string(*perturbationNumber);
                    errMsg += ") is negative";
                    throw Mars2GribDeductionException(errMsg, Here());
                }
                if (*perturbationNumber > numberOfForecastsInEnsemble) {
                    std::string errMsg = "`perturbationNumber` (";
                    errMsg += std::to_string(*perturbationNumber);
                    errMsg += ") is bigger than `numberOfForecastsInEnsemble` (";
                    errMsg += std::to_string(numberOfForecastsInEnsemble);
                    errMsg += ")";
                    throw Mars2GribDeductionException(errMsg, Here());
                }
            }

            // Logging of the par::numberOfForecastsInEnsemble
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`numberOfForecastsInEnsemble` resolved from input dictionaries: value='";
                logMsg += std::to_string(numberOfForecastsInEnsemble);
                logMsg += "'";
                return logMsg;
            }());

            return numberOfForecastsInEnsemble;
        }
        else {

            /// @todo Implement a defaulting strategy for `numberOfForecastsInEnsemble` when the key is missing from the
            /// parameter dictionary.
            std::string errMsg = "Default value for `numberOfForecastsInEnsemble` not implemented";
            throw Mars2GribDeductionException(errMsg, Here());
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `numberOfForecastsInEnsemble` from input dictionaries", Here()));
    };
};


}  // namespace metkit::mars2grib::backend::deductions

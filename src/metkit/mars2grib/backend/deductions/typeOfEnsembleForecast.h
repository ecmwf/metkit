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
/// @file typeOfEnsembleForecast.h
/// @brief Deduction of the GRIB `typeOfEnsembleForecast` identifier.
///
/// This header defines the deduction responsible for resolving the
/// GRIB `typeOfEnsembleForecast` key (Code Table 4.6), which describes
/// the nature of ensemble perturbations.
///
/// The value may be provided explicitly via parametrization or
/// deduced deterministically from MARS metadata.
///
/// Deductions:
/// - extract values from input dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - infer missing values beyond defined rules
/// - apply silent fallbacks
/// - rely on pre-existing GRIB header state
///
/// Error handling follows a strict fail-fast strategy with nested
/// exception propagation to preserve full diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value deduced from input dictionaries
/// - OVERRIDE: explicit user override via parameter dictionary
///
/// @section References
/// Concept:
/// - @ref ensembleEncoding.h
///
/// Related deductions:
/// - @ref perturbationNumber.h
/// - @ref numberOfForecastsInEnsemble.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Tables includes
#include "metkit/mars2grib/backend/tables/typeOfEnsembleForecast.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `typeOfEnsembleForecast` key.
///
/// Resolution follows a strict precedence order:
///
/// 1. **Explicit override**
/// If `par::typeOfEnsembleForecast` is present, its value is taken
/// as authoritative and validated against GRIB Code Table 4.6.
///
/// 2. **Automatic deduction**
/// If no override is provided, the value is deduced from `mars::type`:
/// - `"cf"` → `Unperturbed`
/// - `"pf"` → `Perturbed`
///
/// Any unsupported input results in a deduction failure.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary
/// @param[in] par  Parameter dictionary
/// @param[in] opt  Options dictionary (unused)
///
/// @return The resolved `TypeOfEnsembleForecast` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the value cannot be resolved
///
/// @note
/// This deduction is deterministic and authoritative for
/// `typeOfEnsembleForecast`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::TypeOfEnsembleForecast resolve_TypeOfEnsembleForecast_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                       const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        // Get mars type from dictionary

        if (has(par, "typeOfEnsembleForecast")) {

            // Retrieve mandatory typeOfEnsembleForecast from parameter dictionary
            long typeOfEnsembleForecastVal = get_or_throw<long>(par, "typeOfEnsembleForecast");

            // Get the enum value
            tables::TypeOfEnsembleForecast typeOfEnsembleForecast =
                tables::long2enum_TypeOfEnsembleForecast_or_throw(typeOfEnsembleForecastVal);

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`typeOfEnsembleForecast` overridden from parameter dictionary: value='";
                logMsg += tables::enum2name_TypeOfEnsembleForecast_or_throw(typeOfEnsembleForecast);
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return typeOfEnsembleForecast;
        }
        else {

            // Retrieve mandatory type from MARS dictionary
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

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`typeOfEnsembleForecast` resolved from input dictionaries: value='";
                logMsg += tables::enum2name_TypeOfEnsembleForecast_or_throw(typeOfEnsembleForecast);
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return typeOfEnsembleForecast;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `typeOfEnsembleForecast` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

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
/// @brief Resolve the perturbation (ensemble member) number from the MARS dictionary.
///
/// This deduction retrieves the value associated with the MARS key `number`,
/// which identifies the perturbation (ensemble member) for the encoded field.
///
/// The value is treated as mandatory and is retrieved verbatim from the input
/// dictionaries without inference, defaulting, or transformation.
///
/// @section Deduction contract
/// - Reads: `mars["number"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary; must provide the key `number`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary from which the perturbation number is retrieved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The perturbation number resolved from input dictionaries.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `number` is missing, cannot be converted to `long`, or if any
/// unexpected error occurs during deduction.
///
/// @note
/// This deduction performs no consistency validation against ensemble size.
/// Cross-field constraints are handled by other ensemble deductions.
///
/// @section References
/// Concept:
/// - @ref ensembleEncoding.h
///
/// Related deductions:
/// - @ref numberOfForecastsInEnsemble.h
/// - @ref typeOfEnsembleForecast.h
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the perturbation number (`number`) from the MARS dictionary.
///
/// @section Deduction contract
/// - Reads: `mars["number"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws on error
///
/// This deduction retrieves the perturbation number associated with the
/// current field from the MARS dictionary.
///
/// The value uniquely identifies the ensemble member within an ensemble
/// forecast. Its interpretation (e.g. control vs perturbed members) is
/// handled elsewhere and is not enforced here.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must contain `number`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary from which the perturbation number is retrieved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The perturbation number resolved from the MARS dictionary.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `number` is missing, cannot be converted to `long`, or if
/// any unexpected error occurs during deduction.
///
/// @note
/// This deduction performs no range checks or consistency validation.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_PerturbationNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory perturbation number from input dictionaries
        auto perturbationNumber = get_or_throw<long>(mars, "number");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`number` resolved from input dictionaries: value='";
            logMsg += std::to_string(perturbationNumber);
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return perturbationNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `number` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

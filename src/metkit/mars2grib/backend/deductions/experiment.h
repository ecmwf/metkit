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
/// @file experiment.h
/// @brief Deduction of the MARS `experiment` identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **experiment identifier** from MARS metadata.
///
/// The deduction retrieves the experiment identifier directly from the
/// MARS dictionary and exposes it to the encoding layer without
/// transformation or interpretation.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying minimal, explicit deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply inference, normalization, or defaulting
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or malformed inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value derived via deduction logic from input dictionaries
/// - OVERRIDE: value provided by parameter dictionary overriding deduction logic
///
/// @section References
/// Concept:
/// - @ref destineEncoding.h
///
/// Related deductions:
/// - @ref activity.h
/// - @ref dataset.h
/// - @ref generation.h
/// - @ref model.h
/// - @ref realization.h
/// - @ref resolution.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the experiment identifier from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["experiment"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the experiment identifier by retrieving the
/// mandatory MARS key `experiment` and returning its value as a
/// `std::string`.
///
/// No semantic interpretation, normalization, or validation is applied.
/// The meaning and allowed values of the experiment identifier are
/// defined by upstream MARS conventions.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to
/// `experiment` and conversion to `std::string`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the experiment identifier is resolved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved experiment identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `experiment` is missing, cannot be retrieved as a
/// string, or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction performs presence-only validation and does not
/// consult experiment registries or GRIB tables.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Experiment_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS experiment
        std::string marsExperimentVal = get_or_throw<std::string>(mars, "experiment");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`experiment` resolved from input dictionaries: value='" + marsExperimentVal + "'";
            return logMsg;
        }());

        // Success exit point
        return marsExperimentVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `experiment` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

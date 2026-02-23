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
/// @file resolution.h
/// @brief Deduction of the MARS spatial resolution identifier.
///
/// This header defines the deduction responsible for resolving the
/// spatial resolution identifier from the MARS dictionary.
///
/// The resolved value is passed unchanged to downstream concept
/// encoders and is not interpreted or validated at this stage.
///
/// Deductions:
/// - extract values from MARS, parameter, or option dictionaries
/// - apply deterministic resolution logic
/// - emit structured diagnostic logging
///
/// Deductions do NOT:
/// - perform semantic interpretation
/// - apply defaults or inference
/// - validate GRIB table correctness
///
/// Error handling follows a fail-fast strategy with nested exception
/// propagation to preserve diagnostic context.
///
/// Logging policy:
/// - RESOLVE: value obtained through deduction logic from input dictionaries
///
/// @section References
/// Concept:
/// - @ref destineEncoding.h
///
/// Related deductions:
/// - @ref activity.h
/// - @ref experiment.h
/// - @ref generation.h
/// - @ref model.h
/// - @ref realization.h
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
/// @brief Resolve the spatial resolution identifier from the MARS dictionary.
///
/// @section Deduction contract
/// - Reads: `mars["resolution"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction retrieves the mandatory `resolution` entry from the
/// MARS dictionary and returns it verbatim.
///
/// No inference, defaulting, normalization, or validation of the
/// resolution semantics is performed.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must provide the key `resolution`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary providing the spatial resolution identifier.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// Spatial resolution identifier as provided by the MARS dictionary.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `resolution` is missing, cannot be retrieved as a string,
/// or if any unexpected error occurs.
///
/// @note
/// This deduction is deterministic and does not depend on any
/// pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Resolution_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory resolution identifier from MARS dictionary
        std::string marsResolutionVal = get_or_throw<std::string>(mars, "resolution");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`resolution` resolved from MARS dictionary: value='";
            logMsg += marsResolutionVal;
            logMsg += "'";
            return logMsg;
        }());

        // Success exit point
        return marsResolutionVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `resolution` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

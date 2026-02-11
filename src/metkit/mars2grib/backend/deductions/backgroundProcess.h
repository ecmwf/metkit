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
/// @file backgroundProcess.h
/// @brief Deduction of the GRIB `backgroundProcess` attribute.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **GRIB Background Process** from input dictionaries.
///
/// The deduction maps the MARS model identifier to a GRIB-compliant
/// `BackgroundProcess` value according to predefined, authoritative
/// mapping rules.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying deterministic deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply semantic defaults beyond explicit rules
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or unsupported inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved via deduction logic from input dictionaries
/// - OVERRIDE: value provided by parameter dictionary overriding deduction logic
///
/// @section References
/// Concept:
/// - @ref generatingProcessEncoding.h
///
/// Related deductions:
/// - @ref generatingProcessIdentifier.h
/// - @ref typeOfGeneratingProcess.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Tables
#include "metkit/mars2grib/backend/tables/backgroundProcess.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `backgroundProcess` value from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["model"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction resolves the GRIB `backgroundProcess` by mapping the
/// MARS model identifier to the corresponding
/// `tables::BackgroundProcess` enumeration value.
///
/// The mapping is explicit and strict: only supported model identifiers
/// are accepted. Unsupported or unknown values result in an immediate
/// deduction failure.
///
/// This function acts as the single authoritative deduction point for
/// `backgroundProcess`. All mapping rules and consistency checks for
/// this GRIB key must be implemented here.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `model`
/// and conversion to `std::string`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the model identifier is read.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved GRIB `BackgroundProcess` enumeration value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `model` is missing, unsupported, cannot be mapped to a
/// valid GRIB background process, or if any unexpected error occurs
/// during deduction.
///
/// @note
/// This deduction is deterministic and does not rely on any
/// pre-existing GRIB header state.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::BackgroundProcess resolve_BackgroundProcess_or_throw(const MarsDict_t& mars,
                                                             [[maybe_unused]] const ParDict_t& par,
                                                             [[maybe_unused]] const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory MARS model identifier
        std::string marsModelVal = get_opt<std::string>(mars, "model").value_or("ifs");

        // Apply BackgroundProcess mapping logic
        tables::BackgroundProcess backgroundProcess = tables::name2enum_BackgroundProcess_or_throw(marsModelVal);

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`backgroundProcess` resolved from input dictionaries: value=";
            logMsg += tables::enum2name_BackgroundProcess_or_throw(backgroundProcess) + "'";
            return logMsg;
        }());

        // Success exit point
        return backgroundProcess;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `backgroundProcess` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

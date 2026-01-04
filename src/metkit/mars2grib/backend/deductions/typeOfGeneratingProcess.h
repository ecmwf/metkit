/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file typeOfGeneratingProcess.h
 * @brief Optional deduction of the GRIB `typeOfGeneratingProcess` identifier.
 *
 * This header defines the deduction responsible for *optionally* resolving
 * the GRIB `typeOfGeneratingProcess` key (Code Table 3) from MARS metadata.
 *
 * The deduction is intentionally conservative and returns a value only when
 * a formally defined and unambiguous mapping applies.
 *
 * Deductions:
 * - extract values from input dictionaries
 * - apply deterministic and explicitly defined mappings
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer missing values
 * - apply defaults or fallbacks
 * - guess or approximate generating process semantics
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: supported mapping applied
 * - RESOLVE (skip): deduction intentionally not applied
 *
 * @section References
 * Concept:
 *   - @ref generatingProcessEncoding.h
 *
 * Related deductions:
 *   - @ref generatingProcessIdentifier.h
 *   - @ref backgroundProcess.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <optional>
#include <string>

// Tables includes
#include "metkit/mars2grib/backend/tables/typeOfGeneratingProcess.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Optionally resolve the GRIB `typeOfGeneratingProcess` key.
 *
 * This deduction attempts to infer the GRIB
 * `typeOfGeneratingProcess` value from MARS metadata.
 *
 * The deduction is **non-mandatory** and applies only when a formally
 * specified and explicitly supported mapping is identified.
 * If no such mapping exists, the deduction returns `std::nullopt`
 * without raising an error.
 *
 * @section Current deduction logic
 * - If `mars::type == "4i"`, the generating process is resolved as
 *   `AnalysisIncrement`.
 * - All other cases result in `std::nullopt`.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary providing metadata used for deduction
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return
 * - `tables::TypeOfGeneratingProcess` if the deduction applies
 * - `std::nullopt` if no supported deduction is identified
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If required MARS keys are missing or dictionary access fails
 *
 * @note
 * This deduction does not rely on pre-existing GRIB header state and
 * does not apply defaults.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<tables::TypeOfGeneratingProcess> resolve_TypeOfGeneratingProcess_opt(
    const MarsDict_t& mars, [[maybe_unused]] const ParDict_t& par, [[maybe_unused]] const OptDict_t& opt) {

    using metkit::mars2grib::backend::tables::TypeOfGeneratingProcess;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    // N.B. Sometimes this is overwritten by eccodes as a side effect of setting `param`
    try {

        // Retrieve mandatory type from MARS dictionary
        std::string marsTypeVal = get_or_throw<std::string>(mars, "type");
        // std::string marsStreamVal = get_or_throw<std::string>(mars, "stream");
        // std::string marsClassVal = get_or_throw<std::string>(mars, "class");
        // long paramIdVal = get_or_throw<long>(mars, "param");

        // Deduce the typeOfGeneratingProcess
        if (marsTypeVal == "4i") {

            tables::TypeOfGeneratingProcess result = TypeOfGeneratingProcess::AnalysisIncrement;

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`typeOfGeneratingProcess` resolved from input dictionaries: value='";
                logMsg += tables::enum2name_TypeOfGeneratingProcess_or_throw(result);
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return {result};
        }
        else {

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg =
                    "`typeOfGeneratingProcess` not resolved from input dictionaries: no supported mapping";
                return logMsg;
            }());

            // Success exit point
            return std::nullopt;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `typeOfGeneratingProcess` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

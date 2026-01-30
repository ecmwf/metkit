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
 * @file periodItMin.h
 * @brief Deduction of the minimum wave period index (`iTmin`).
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to retrieve the **minimum wave period index** (`iTmin`) from input
 * dictionaries.
 *
 * The deduction treats `iTmin` as an optional parameter:
 * - if present in the parameter dictionary, the value is returned
 * - if absent, no default is applied and an empty optional is returned
 *
 * No semantic validation or consistency checking is performed at this
 * level.
 *
 * Error handling follows a strict fail-fast strategy:
 * - unexpected access errors cause immediate failure
 * - errors are reported using domain-specific deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value presence or absence resolved from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref waveEncoding.h
 *
 * Related deductions:
 *   - @ref periodItMax.h
 *   - @ref waveFrequencyGrid.h
 *   - @ref waveFrequencyNumber.h
 *   - @ref waveDirectionGrid.h
 *   - @ref waveDirectionNumber.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <optional>
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the optional minimum wave period index (`iTmin`).
 *
 * @section Deduction contract
 * - Reads: `par["iTmin"]` (optional)
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws on unexpected errors
 *
 * This deduction retrieves the optional wave period index `iTmin`
 * from the parameter dictionary.
 *
 * If the key is present, the value is returned wrapped in a
 * `std::optional`. If the key is absent, an empty optional is returned.
 * No defaulting, inference, or semantic validation is applied.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary. May contain `iTmin`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary from which `iTmin` may be retrieved.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   An optional containing `iTmin` if present; otherwise an empty
 *   optional.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If any unexpected error occurs during dictionary access.
 *
 * @note
 *   This deduction performs no semantic validation of the retrieved
 *   value.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_PeriodItMin_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve optional minimum wave period index from parameter dictionary
        std::optional<long> itMinOpt = get_opt<long>(par, "iTmin");

        if (itMinOpt.has_value()) {
            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`iTmin` resolved from input dictionaries: value='";
                logMsg += std::to_string(itMinOpt.value());
                logMsg += "'";
                return logMsg;
            }());
        }
        else {
            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`iTmin` resolved from input dictionaries: value not present";
                return logMsg;
            }());
        }

        // Success exit point
        return itMinOpt;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `iTmin` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

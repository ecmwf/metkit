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
 * @file derivedForecast.h
 * @brief Deduction of the GRIB `derivedForecast` key.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **GRIB derived forecast type** (GRIB2 Code Table 4.7).
 *
 * The deduction supports both explicit user override and automatic
 * deduction from MARS metadata, following a strict precedence order.
 *
 * Deductions are responsible for:
 * - extracting values from MARS, parameter, and option dictionaries
 * - applying explicit validation and mapping logic
 * - returning strongly typed GRIB table values to concept operations
 *
 * Deductions:
 * - do NOT encode GRIB keys directly
 * - do NOT apply implicit defaults outside explicitly defined rules
 * - do NOT perform GRIB table validation beyond table lookups
 *
 * Error handling follows a strict fail-fast strategy:
 * - missing or unsupported inputs cause immediate failure
 * - errors are reported using domain-specific deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value derived via deduction logic from input dictionaries
 * - OVERRIDE: value provided by parameter dictionary overriding deduction logic
 *
 * @section References
 * Concept:
 *   - @ref derivedEncoding.h
 *
 * Related deductions:
 *   - @ref numberOfForecastsInEnsemble.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// Tables
#include "metkit/mars2grib/backend/tables/derivedForecast.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `derivedForecast` key.
 *
 * @section Deduction contract
 * - Reads:
 *   - `par["derivedForecast"]` (if present),
 *   - otherwise `mars["type"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE or OVERRIDE)
 * - Failure mode: throws
 *
 * This deduction resolves the GRIB `derivedForecast` value following a
 * strict precedence order.
 *
 * If the parameter dictionary provides `derivedForecast`, the value is
 * treated as authoritative and validated via the GRIB code table.
 * Otherwise, the value is intended to be deduced from the MARS key
 * `type`.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide `type` if automatic
 *   deduction is required.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary. May contain `derivedForecast`
 *   as a numeric override.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary providing metadata for automatic deduction.
 *
 * @param[in] par
 *   Parameter dictionary optionally providing `derivedForecast`.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The resolved `tables::DerivedForecast` enumeration value.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If an override value is invalid, if required MARS metadata is
 *   missing, if automatic deduction is not supported, or if any
 *   unexpected error occurs.
 *
 * @note
 *   Automatic deduction from `mars["type"]` is currently incomplete
 *   and will fail for all inputs until implemented.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::DerivedForecast resolve_DerivedForecast_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                         const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Default to Missing
        tables::DerivedForecast derivedForecast = tables::DerivedForecast::Missing;

        if (has(par, "derivedForecast")) {

            // Retrieve override from parameter dictionary
            long derivedForecastVal = get_or_throw<long>(par, "derivedForecast");

            // Lookup and validate enum value
            derivedForecast = tables::long2enum_DerivedForecast_or_throw(derivedForecastVal);

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`derivedForecast` resolved from input dictionaries: value='";
                logMsg += tables::enum2name_DerivedForecast_or_throw(derivedForecast);
                logMsg += "'";
                return logMsg;
            }());
        }
        else {

            // Retrieve type from mars dictionary
            std::string marsType = get_or_throw<std::string>(mars, "type");

            // TODO MIVAL: Implement automatic deduction from mars.type --- IGNORE ---
            throw Mars2GribDeductionException("Not implemented", Here());

            // // Emit RESOLVE log entry
            // MARS2GRIB_LOG_RESOLVE([&]() {
            //     std::string logMsg = "`derivedForecast` resolved from input dictionaries: value='";
            //     logMsg += tables::enum2name_DerivedForecast_or_throw(derivedForecast);
            //     logMsg += "'";
            //     return logMsg;
            // }());
        }

        // Success exit point
        return derivedForecast;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `derivedForecast` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

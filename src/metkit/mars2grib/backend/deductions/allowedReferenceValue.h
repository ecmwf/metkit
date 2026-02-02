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
 * @file allowedReferenceValue.h
 * @brief Deduction of the GRIB `allowedReferenceValue`.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **GRIB allowed reference value** associated with a given
 * MARS parameter identifier.
 *
 * The deduction retrieves the MARS parameter code from the input dictionaries
 * and derives a representative reference value based on a statically defined
 * set of admissible value ranges.
 *
 * Deductions are responsible for:
 * - extracting values from MARS, parameter, and option dictionaries
 * - applying minimal, explicit deduction logic
 * - returning strongly typed values to concept operations
 *
 * Deductions:
 * - do NOT encode GRIB keys directly
 * - do NOT apply inference, normalization, or defaulting beyond explicit rules
 * - do NOT perform GRIB table validation
 *
 * Error handling follows a strict fail-fast strategy:
 * - missing or malformed inputs cause immediate failure
 * - errors are reported using domain-specific deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value derived via deduction logic from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref marsEncoding.h
 *
 * Related deductions:
 *   - @ref class.h
 *   - @ref expver.h
 *   - @ref stream.h
 *   - @ref type.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>
#include <unordered_map>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB allowed reference value from input dictionaries.
 *
 * @section Deduction contract
 * - Reads: `mars["param"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE)
 * - Failure mode: throws
 *
 * This deduction resolves the GRIB `allowedReferenceValue` by retrieving
 * the mandatory MARS parameter identifier (`param`) and consulting a
 * statically defined table of admissible reference value ranges.
 *
 * For parameters with an explicit range definition, the resolved reference
 * value is chosen as the midpoint of the corresponding `[min, max]` interval.
 * If no explicit range is defined for the parameter, a default reference
 * value of `0.0` is returned.
 *
 * No semantic interpretation beyond the explicit range table is applied.
 * The admissible ranges are defined locally and are not validated against
 * external GRIB tables.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must support keyed access to `param`
 *   and conversion to an integral type.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the parameter identifier is resolved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The resolved allowed reference value.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If the key `param` is missing, cannot be retrieved as an integral value,
 *   or if any unexpected error occurs during deduction.
 *
 * @note
 *   This deduction applies a local, table-driven rule and does not
 *   consult GRIB metadata tables or apply parameter-specific semantics.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
double resolve_AllowedReferenceValue_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Static map of allowed reference value ranges keyed by parameter code
        static const std::unordered_map<long, std::pair<double, double>> param_ranges = {
            {3, {170.0, 1200.0}},
            {10, {0.0, 300.0}},
            {31, {-0.00001, 1.001}},
            {33, {10.0, 1000.0}},
            {34, {160.0, 320.0}},
            {43, {0.0, 10.0}},
            {49, {0.0, 100.0}},
            {54, {100.0, 108000.0}},
            {59, {0.0, 40000.0}},
            {60, {-1.0, 1.0}},
            {121, {160.0, 380.0}},
            {122, {150.0, 330.0}},
            {129, {-13000.0, 3500000.0}},
            {130, {140.0, 400.0}},
            {131, {-250.0, 250.0}},
            {132, {-250.0, 250.0}},
            {133, {-0.1, 0.1}},
            {134, {43000.0, 115000.0}},
            {135, {-30.0, 30.0}},
            {136, {-50.0, 220.0}},
            {151, {85000.0, 125000.0}},
            {156, {-1300.0, 35000.0}},
            {157, {0.0, 180.0}},
            {164, {0.0, 1.0}},
            {165, {-150.0, 150.0}},
            {166, {-100.0, 100.0}},
            {167, {160.0, 370.0}},
            {168, {25.0, 350.0}},
            {172, {0.0, 1.0}},
            {173, {0.0, 10.0}},
            {186, {0.0, 1.0}},
            {187, {0.0, 1.0}},
            {188, {0.0, 1.0}},
            {207, {0.0, 300.0}},
            {235, {120.0, 380.0}},
            {246, {-0.001, 1e6}},
            {247, {-0.001, 0.01}},
            {3031, {0.0, 360.1}},
            {3062, {-0.05, 130.0}},
            {3066, {0.0, 5.0}},
            {3073, {0.0, 100.0}},
            {3074, {0.0, 100.0}},
            {3075, {0.0, 100.0}},
            {140230, {-1.0, 360.5}},
            {151131, {-3.5, 3.5}},
            {151132, {-3.5, 3.5}},
            {151145, {-4.0, 4.0}},
            {228001, {-60000.0, 1000.0}},
            {228002, {-1300.0, 8888.0}},
            {228004, {160.0, 370.0}},
            {228005, {0.0, 300.0}},
            {228006, {0.0, 1.0}},
            {228141, {-1e-10, 15000.0}},
            {260057, {-3.0, 150.0}},
            {260259, {-10.0, 5.0}},
            {260260, {0.0, 360.1}},
            {262101, {160.0, 320.0}},
            {262140, {-3.5, 3.5}},
            {262501, {173.0, 1000.0}},
            {263101, {160.0, 320.0}},
            {263140, {-3.5, 3.5}},
            {263501, {173.0, 1000.0}},
        };

        // Default reference value
        double ret = 0.0;

        // Retrieve mandatory MARS allowedReferenceValue
        long marsParamVal = get_or_throw<long>(mars, "param");

        // Lookup allowed value in the mid of the allowed range
        if (auto rangeIt = param_ranges.find(marsParamVal); rangeIt != param_ranges.end()) {
            const auto& [minVal, maxVal] = rangeIt->second;
            ret                          = 0.5 * (minVal + maxVal);
        }


        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg =
                "`allowedReferenceValue` resolved from input dictionaries: value='" + std::to_string(ret) + "'";
            return logMsg;
        }());

        // Success exit point
        return ret;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `allowedReferenceValue` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
 * @file bitsPerValue.h
 * @brief Deduction of the GRIB `bitsPerValue` packing parameter.
 *
 * This header defines deduction utilities used by the mars2grib backend
 * to resolve the **GRIB bits-per-value packing parameter** used by the
 * Data Representation Section.
 *
 * The deduction logic is **explicitly split by data representation**:
 * - **Gridded data** use a metadata-driven default mapping derived from
 *   legacy MultIO behavior, with optional user override.
 * - **Spectral data** use a fixed default packing precision, with
 *   optional user override.
 *
 * This separation is intentional and reflects fundamentally different
 * packing policies for gridded and spectral fields. The two code paths
 * are implemented as distinct deductions to avoid hidden coupling,
 * conditional complexity, and ambiguous semantics.
 *
 * Deductions are responsible for:
 * - extracting values from MARS, parameter, and option dictionaries
 * - applying deterministic deduction and validation logic
 * - returning strongly typed values to concept operations
 *
 * Deductions:
 * - do NOT encode GRIB keys directly
 * - do NOT apply heuristic or data-driven inference
 * - do NOT perform GRIB table validation
 *
 * Error handling follows a strict fail-fast strategy:
 * - missing or invalid inputs cause immediate failure
 * - errors are reported using domain-specific deduction exceptions
 * - original errors are preserved via nested exception propagation
 *
 * Logging follows the mars2grib deduction policy:
 * - RESOLVE: value derived via deduction logic from input dictionaries
 * - OVERRIDE: value provided by parameter dictionary overriding deduction logic
 *
 * @section References
 * Concept:
 *   - @ref packingEncoding.h
 *
 * Related deductions:
 *   - @ref laplacianOperator.h
 *   - @ref subSetTrunc.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {


namespace details {

/**
 * @brief Determine the default GRIB packing precision (bitsPerValue).
 *
 * This function returns the number of bits per value used for GRIB
 * data packing based on:
 *  - the GRIB parameter identifier (`paramId`)
 *  - the vertical level prefix (`prefix`)
 *  - whether compression is enabled (`enableCompression`)
 *
 * The logic implemented here is directly derived from the legacy function
 * `LOOKUP_BITS_PER_VALUE_DEFAULT` code path in **MultIO**, and is preserved to ensure
 * bit-for-bit compatibility with existing production workflows.
 *  - <multio-src>/src/multiom/ifs2mars/ifs2mars/ifs2mars_mod.F90
 *
 * @param paramId
 *   GRIB parameter identifier (discipline/category/number flattened).
 *
 * @param prefix
 *   Vertical level prefix:
 *   - `"pl"` : pressure levels
 *   - `"ml"` : model levels
 *
 * @param enableCompression
 *   If true, reduced precision may be applied for model-level fields.
 *
 * @return
 *   Number of bits per value to be used for GRIB packing.
 *
 * @details
 * Decision logic (evaluated in order):
 *
 * - `paramId == 248` (Cloud cover)
 *     → 8 bits
 *
 * - `paramId == 141` (backward compatibility),
 *   `paramId == 228141` (Snow depth),
 *   `paramId == 244` (Forecast surface roughness)
 *     → 24 bits
 *
 * - `paramId == 246` (Cloud liquid water content) on pressure levels
 *     → 12 bits
 *
 * - `paramId == 247` (Cloud ice water content) on pressure levels
 *     → 12 bits
 *
 * - `210000 < paramId < 228000`
 *     → 24 bits
 *
 * - `paramId == 260510` (Cloudy brightness temperature),
 *   `paramId == 260511` (Clear-sky brightness temperature)
 *     → 10 bits
 *
 * - Compression enabled on model levels
 *     → 10 bits
 *
 * - Default case
 *     → 16 bits
 *
 * @note
 * The function is purely deterministic and has no side effects.
 * No validation of `paramId` or `prefix` is performed.
 *
 * @note
 * The logic is not 100% identical to the original MultIO code path.
 * Logic for parameters between 80 and 120 has been removed, as it
 * requires additional parameters, and in this case the override can
 * be used anyway.
 */
long lookup_bitsPerValueGridded_default(long paramId, std::string prefix, bool enableCompression) {

    // Parameter IDs (hard-coded)
    constexpr long CLOUD_COVER                      = 248;
    constexpr long SNOW_DEPTH_BACKWARD_COMPAT       = 141;
    constexpr long SNOW_DEPTH                       = 228141;
    constexpr long FORECAST_SURFACE_ROUGHNESS       = 244;
    constexpr long CLOUD_LIQUID_WATER_CONTENT       = 246;
    constexpr long CLOUD_ICE_WATER_CONTENT          = 247;
    constexpr long CLOUDY_BRIGHTNESS_TEMPERATURE    = 260510;
    constexpr long CLEAR_SKY_BRIGHTNESS_TEMPERATURE = 260511;

    if (paramId == CLOUD_COVER) {
        return 8;
    }
    else if (paramId == SNOW_DEPTH_BACKWARD_COMPAT || paramId == SNOW_DEPTH || paramId == FORECAST_SURFACE_ROUGHNESS) {
        return 24;
    }
    else if (paramId == CLOUD_LIQUID_WATER_CONTENT && prefix == "pl") {
        return 12;
    }
    else if (paramId == CLOUD_ICE_WATER_CONTENT && prefix == "pl") {
        return 12;
    }
    else if (paramId > 210000 && paramId < 228000) {
        return 24;
    }
    else if (paramId == CLOUDY_BRIGHTNESS_TEMPERATURE || paramId == CLEAR_SKY_BRIGHTNESS_TEMPERATURE) {
        return 10;
    }
    else if (enableCompression && prefix == "ml") {
        return 10;
    }
    else {
        return 16;
    }
}

}  // namespace details


/**
 * @brief Resolve the GRIB `bitsPerValue` packing parameter for gridded data.
 *
 * @section Deduction contract
 * - Reads:
 *   - `par["bitsPerValue"]` (if present),
 *   - otherwise `mars["param"]`, `mars["levtype"]`,
 *     and `opt["enableBitsPerValueCompression"]`
 * - Writes: none
 * - Side effects: logging (RESOLVE or OVERRIDE)
 * - Failure mode: throws
 *
 * This deduction resolves the number of bits per value used for GRIB
 * numeric packing of **gridded fields**.
 *
 * If `bitsPerValue` is explicitly provided in the parameter dictionary,
 * it is taken verbatim and overrides any default deduction logic.
 * Otherwise, a deterministic default mapping is applied based on
 * MARS metadata and encoder options.
 *
 * The default mapping logic is delegated to
 * `details::lookup_bits_per_value_default` and is designed to preserve
 * bit-for-bit compatibility with legacy MultIO workflows.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary. Must provide `paramId` and `levtype`
 *   if default deduction is required.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary. May contain `bitsPerValue`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary. May contain
 *   `enableBitsPerValueCompression`.
 *
 * @param[in] mars
 *   MARS dictionary providing metadata for default deduction.
 *
 * @param[in] par
 *   Parameter dictionary optionally providing `bitsPerValue`.
 *
 * @param[in] opt
 *   Options dictionary controlling default deduction behavior.
 *
 * @return
 *   The resolved number of bits per value to be used for GRIB packing
 *   of gridded data.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If required inputs are missing, if the resolved value is outside
 *   the supported range, or if any unexpected error occurs.
 *
 * @note
 *   This deduction enforces strict numeric validation but does not
 *   attempt to optimize packing precision based on data statistics
 *   or parameter semantics.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_BitsPerValueGridded_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        long bitsPerValue;
        if (has(par, "bitsPerValue")) {

            // Retrieve mandatory bitsPerValue from parameter dictionary
            bitsPerValue = get_or_throw<long>(par, "bitsPerValue");

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`bitsPerValue` overridden by parameter dictionary: value=";
                logMsg += std::to_string(bitsPerValue) + "'";
                return logMsg;
            }());
        }
        else {

            // Retrive auxiliary values for default lookup
            long paramId          = get_or_throw<long>(mars, "param");
            std::string levtype   = get_or_throw<std::string>(mars, "levtype");
            bool applyCompression = get_opt<bool>(opt, "enableBitsPerValueCompression").value_or(false);

            // Resolve bitsPerValue from default mapping
            bitsPerValue = details::lookup_bitsPerValueGridded_default(paramId, levtype, applyCompression);

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`bitsPerValue` resolved from input dictionaries: value=";
                logMsg += std::to_string(bitsPerValue) + "'";
                return logMsg;
            }());
        }

        // Validate bits per value
        if (bitsPerValue < 0 || bitsPerValue > 64) {
            std::string errMsg = "Invalid `bitsPerValue`: value='";
            errMsg += std::to_string(bitsPerValue) + "'";
            throw Mars2GribDeductionException(errMsg, Here());
        }

        // Success exit point
        return bitsPerValue;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `bitsPerValue` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


/**
 * @brief Resolve the GRIB `bitsPerValue` packing parameter for spectral data.
 *
 * @section Deduction contract
 * - Reads: `par["bitsPerValue"]` (if present)
 * - Writes: none
 * - Side effects: logging (RESOLVE or OVERRIDE)
 * - Failure mode: throws
 *
 * This deduction resolves the number of bits per value used for GRIB
 * numeric packing of **spectral fields**.
 *
 * If `bitsPerValue` is explicitly provided in the parameter dictionary,
 * it is taken verbatim and overrides any deduction logic.
 * Otherwise, a fixed default value of `16` bits is applied.
 *
 * No MARS metadata is consulted for spectral packing.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused by this deduction).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary. May contain `bitsPerValue`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary optionally providing `bitsPerValue`.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The resolved number of bits per value to be used for GRIB packing
 *   of spectral data.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If the resolved value is outside the supported range or if any
 *   unexpected error occurs.
 *
 * @note
 *   This deduction applies a fixed default packing precision for
 *   spectral data and enforces strict numeric validation.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_BitsPerValueSpectral_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        long bitsPerValue;
        if (has(par, "bitsPerValue")) {

            // Retrieve mandatory bitsPerValue from parameter dictionary
            bitsPerValue = get_or_throw<long>(par, "bitsPerValue");

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`bitsPerValue` overridden by parameter dictionary: value=";
                logMsg += std::to_string(bitsPerValue) + "'";
                return logMsg;
            }());
        }
        else {

            // Resolve bitsPerValue from default mapping
            bitsPerValue = 16;

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`bitsPerValue` resolved from input dictionaries: value=";
                logMsg += std::to_string(bitsPerValue) + "'";
                return logMsg;
            }());
        }

        // Validate bits per value
        if (bitsPerValue < 0 || bitsPerValue > 64) {
            std::string errMsg = "Invalid `bitsPerValue`: value='";
            errMsg += std::to_string(bitsPerValue) + "'";
            throw Mars2GribDeductionException(errMsg, Here());
        }

        // Success exit point
        return bitsPerValue;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `bitsPerValue` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

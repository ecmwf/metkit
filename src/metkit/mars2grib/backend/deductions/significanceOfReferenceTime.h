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
 * @file significanceOfReferenceTime.h
 * @brief Deduction of the GRIB `significanceOfReferenceTime` identifier.
 *
 * This header defines the deduction responsible for resolving the
 * GRIB `significanceOfReferenceTime` key, which describes the semantic
 * meaning of the GRIB reference time.
 *
 * The value is deterministically deduced from the MARS request,
 * specifically from the `mars::type` key, according to established
 * ECMWF/MARS conventions.
 *
 * Deductions:
 * - extract values from input dictionaries
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer missing values
 * - apply defaults or fallbacks
 * - validate against external GRIB code tables beyond explicit mappings
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - RESOLVE: value deduced deterministically from input dictionaries
 *
 * @section References
 * Concept:
 *   - @ref referenceTimeEncoding.h
 *
 * Related deductions:
 *   - @ref standardReferenceDateTime.h
 *   - @ref hindcastReferenceDateTime.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <algorithm>
#include <string>

// Tables includes
#include "metkit/mars2grib/backend/tables/significanceOfReferenceTime.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `significanceOfReferenceTime` key.
 *
 * This deduction resolves the GRIB `significanceOfReferenceTime`
 * identifier based on the value of the MARS key `type`.
 *
 * Resolution rules:
 * - Analysis-like MARS types map to
 *   `SignificanceOfReferenceTime::Analysis`
 * - Forecast-like MARS types map to
 *   `SignificanceOfReferenceTime::ForecastStart`
 *
 * The mapping is explicit and exhaustive. Any unsupported MARS
 * `type` value results in a deduction error.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary; must contain the key `type`
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved `SignificanceOfReferenceTime` enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - `mars::type` is missing
 *         - `mars::type` cannot be mapped to a supported significance
 *         - any unexpected error occurs during deduction
 *
 * @note
 * This deduction is fully deterministic and does not rely on any
 * pre-existing GRIB header state.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::SignificanceOfReferenceTime resolve_SignificanceOfReferenceTime_or_throw(const MarsDict_t& mars,
                                                                                 const ParDict_t& par,
                                                                                 const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        auto significanceOfReferenceTime = tables::SignificanceOfReferenceTime::Missing;

        // Retrieve mandatory type from Mars dictionary
        auto marsType = get_or_throw<std::string>(mars, "type");

        constexpr std::array<std::string_view, 17> analysisTypes = {
            {"an", "ia", "oi", "3v", "3g", "4g", "ea", "pa", "tpa", "ga", "gai", "ai", "af", "ab", "oai", "ga", "gai"}};

        constexpr std::array<std::string_view, 33> forecastTypes = {
            {"fc",     "cf",    "pf",    "cm",      "fp",  "em",  "ep",   "es",   "fa",     "efi", "efic",
             "bf",     "cd",    "wem",   "wes",     "cr",  "ses", "taem", "taes", "sg",     "sf",  "if",
             "fcmean", "fcmax", "fcmin", "fcstdev", "ssd", "tf",  "bf",   "cd",   "hcmean", "s3",  "si"}};

        constexpr std::array<std::string_view, 4> startOfDataAssimilationTypes = {{"4i", "4v", "me", "eme"}};

        if (std::any_of(analysisTypes.begin(), analysisTypes.end(), [&marsType](auto v) { return marsType == v; })) {
            significanceOfReferenceTime = tables::SignificanceOfReferenceTime::Analysis;
        }
        else if (std::any_of(forecastTypes.begin(), forecastTypes.end(),
                             [&marsType](auto v) { return marsType == v; })) {
            significanceOfReferenceTime = tables::SignificanceOfReferenceTime::ForecastStart;
        }
        else if (std::any_of(startOfDataAssimilationTypes.begin(), startOfDataAssimilationTypes.end(),
                             [&marsType](auto v) { return marsType == v; })) {
            significanceOfReferenceTime = tables::SignificanceOfReferenceTime::AssimilationStart;
        }
        else {
            // Unhandled cases
            throw Mars2GribDeductionException(
                "Failed to resolve `significanceOfReferenceTime` from MARS type: " + marsType, Here());
        }

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`significanceOfReferenceTime` resolved from input dictionaries: value='";
            logMsg += tables::enum2name_SignificanceOfReferenceTime_or_throw(significanceOfReferenceTime);
            logMsg += "'";
            return logMsg;
        }());

        /// Success exit point
        return significanceOfReferenceTime;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `significanceOfReferenceTime` from input dictionaries", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

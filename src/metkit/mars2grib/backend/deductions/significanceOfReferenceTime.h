/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <algorithm>
#include <array>
#include <string>

#include "eckit/log/Log.h"

#include "metkit/mars2grib/backend/tables/significanceOfReferenceTime.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Deduce the GRIB significance of reference time from the MARS type.
 *
 * This deduction determines the GRIB concept *Significance of Reference Time*
 * based on the value of the MARS key `type`. The MARS type is interpreted
 * according to established ECMWF/MARS conventions and mapped onto the
 * corresponding GRIB significance enumeration.
 *
 * The deduction logic follows these rules:
 * - If the MARS `type` corresponds to an *analysis-like* product
 *   (e.g. analysis, increment, variational analysis, ensemble analysis),
 *   the significance is set to
 *   `tables::SignificanceOfReferenceTime::Analysis`.
 * - If the MARS `type` corresponds to a *forecast-like* product
 *   (e.g. forecast, control forecast, ensemble forecast, probability forecast),
 *   the significance is set to
 *   `tables::SignificanceOfReferenceTime::ForecastStart`.
 * - If the MARS `type` cannot be classified into one of the supported
 *   categories, the deduction fails with an exception.
 *
 * The resulting value is logged in human-readable form using the GRIB
 * table name associated with the enumeration.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `type`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the `type` key is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The deduced significance of reference time, expressed as a value of
 *   `tables::SignificanceOfReferenceTime`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `type` is not present in the MARS dictionary,
 *   - the MARS `type` value cannot be mapped to a supported significance
 *     of reference time,
 *   - any unexpected error occurs during deduction.
 *
 * @note
 *   The mapping between MARS `type` values and GRIB significance categories
 *   is based on ECMWF operational conventions and mirrors long-standing
 *   behavior in the MARS → GRIB conversion chain.
 *
 * @note
 *   The list of recognized analysis and forecast types is intentionally
 *   explicit. New MARS types must be added to the corresponding lookup
 *   tables to be supported.
 *
 * @note
 *   This deduction follows a fail-fast strategy and uses nested exception
 *   propagation to preserve full error provenance across API boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::SignificanceOfReferenceTime resolve_SignificanceOfReferenceTime_or_throw(const MarsDict_t& mars,
                                                                                 const ParDict_t& par,
                                                                                 const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        tables::SignificanceOfReferenceTime result = tables::SignificanceOfReferenceTime::Missing;

        // Get the mars.type
        auto marsType = get_or_throw<std::string>(mars, "type");

        constexpr std::array<std::string_view, 19> analysisTypes = {{"an", "ia", "oi", "3v", "4v", "3g", "4g", "ea",
                                                                     "4i", "pa", "tpa", "ga", "gai", "ai", "af", "ab",
                                                                     "oai", "ga", "gai"}};

        constexpr std::array<std::string_view, 33> forecastTypes = {
            {"fc",     "cf",    "pf",    "cm",      "fp",  "em",  "es",   "fa",   "efi",    "efic", "bf",
             "cd",     "me",    "wem",   "wes",     "cr",  "ses", "taem", "taes", "sg",     "sf",   "if",
             "fcmean", "fcmax", "fcmin", "fcstdev", "ssd", "tf",  "bf",   "cd",   "hcmean", "s3",   "si"}};

        if (std::any_of(analysisTypes.begin(), analysisTypes.end(), [&marsType](auto v) { return marsType == v; })) {
            result = tables::SignificanceOfReferenceTime::Analysis;
        }
        else if (std::any_of(forecastTypes.begin(), forecastTypes.end(),
                             [&marsType](auto v) { return marsType == v; })) {
            result = tables::SignificanceOfReferenceTime::ForecastStart;
        }
        else {
            // Unhandled cases
            throw Mars2GribDeductionException(
                "Unable to deduce `significanceOfReferenceTime` from MARS type: " + marsType, Here());
        }

        // Logging of the resolution
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "significanceOfReferenceTime: deduced from mars dictionary with value: ";
            logMsg += tables::enum2name_SignificanceOfReferenceTime_or_throw(result);
            return logMsg;
        }());

        // Exit with the result
        return result;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Could not deduce `significanceOfReferenceTime` from MARS dictionaries", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

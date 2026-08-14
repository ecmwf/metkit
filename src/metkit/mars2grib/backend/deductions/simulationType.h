/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file simulationType.h
/// @brief Public deduction header for the normalized ProductTimeSpec simulation type.
///
/// Exposes `resolve_SimulationType_or_throw`, the canonical entry point that
/// derives the normalized ProductTimeSpec simulation type from the raw MARS
/// `type` key.
///
/// This deduction owns:
/// - direct `type` dictionary access;
/// - the ProductTimeSpec-specific whitelist that classifies MARS type values as
///   forecast-like or analysis-like.
///
/// This deduction does NOT:
/// - classify simulation regime;
/// - construct ProductTimeSpec model artifacts;
/// - factor its whitelist with other deductions at this stage.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the normalized ProductTimeSpec simulation type.
///
/// @section Deduction contract
///   - Reads (MARS): `type`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - analysis-like MARS `type` values -> `SimulationType::Analysis`;
/// - forecast-like MARS `type` values -> `SimulationType::Forecast`;
/// - start-of-data-assimilation MARS `type` values -> `SimulationType::Analysis`;
/// - any other value -> hard error.
///
/// The whitelist is intentionally kept identical to the current ProductTimeSpec
/// model-local logic.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `type`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return The normalized ProductTimeSpec simulation type.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on missing, malformed, unrecognized, or unsupported raw `type`
///         input, with the original cause attached via
///         `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
SimulationType resolve_SimulationType_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    constexpr std::array<std::string_view, 17> analysisTypes = {
        {"an", "ia", "oi", "3v", "3g", "4g", "ea", "pa", "tpa", "ga", "gai", "ai", "af", "ab", "oai", "ga", "gai"}};
    constexpr std::array<std::string_view, 40> forecastTypes = {
        {"fc",     "cf", "pf",     "cm",    "fp",    "em",      "ep",  "es",   "fa",   "efi",
         "efic",   "bf", "cd",     "wem",   "wes",   "cr",      "ses", "taem", "taes", "sg",
         "sf",     "if", "fcmean", "fcmax", "fcmin", "fcstdev", "ssd", "tf",   "bf",   "cd",
         "hcmean", "s3", "si",     "gbf",   "gwt",   "est",     "icp", "pfc",  "sot",  "4v"}};
    constexpr std::array<std::string_view, 3> startOfDataAssimilationTypes = {{"4i", "me", "eme"}};

    try {
        const std::string type = get_or_throw<std::string>(mars, "type");

        const bool isAnalysis =
            std::any_of(analysisTypes.begin(), analysisTypes.end(), [&type](auto value) { return type == value; });
        const bool isForecast =
            std::any_of(forecastTypes.begin(), forecastTypes.end(), [&type](auto value) { return type == value; });
        const bool isAssimilationStart =
            std::any_of(startOfDataAssimilationTypes.begin(), startOfDataAssimilationTypes.end(),
                        [&type](auto value) { return type == value; });

        if (!isAnalysis && !isForecast && !isAssimilationStart) {
            throw Mars2GribDeductionException(
                "Failed to resolve `simulationType`: MARS dictionary has unrecognized value for key `type`: " + type,
                Here());
        }

        const SimulationType result =
            (isAnalysis || isAssimilationStart) ? SimulationType::Analysis : SimulationType::Forecast;
        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`simulationType` resolved from input dictionaries: value='"} +
                   (result == SimulationType::Analysis ? "Analysis" : "Forecast") + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `simulationType` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

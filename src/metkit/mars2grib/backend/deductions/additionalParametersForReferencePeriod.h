// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file additionalParametersForReferencePeriod.h
/// @brief Deduction of the additional parameters of the GRIB reference period.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **additional parameters of the reference period**
/// (`scaleFactorOfAdditionalParameterForReferencePeriod` and
/// `scaledValueOfAdditionalParameterForReferencePeriod`) of SOT products.
///
/// The MARS `quantile` of a SOT product has the form `<low>-<high>:<denominator>`,
/// with denominator 100 * 10^k (e.g. `90-99:100`, `1-10:100`, `900-995:1000`).
/// The percentile nearer to the median comes first (SOT90 = {90, 99},
/// SOT10 = {10, 1}, the order ecCodes uses to build `mars.quantile`, ECC-2000),
/// and each value uses the smallest exact scale factor.
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or malformed inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value derived via deduction logic from input dictionaries
///
/// @section References
/// Concept:
/// - @ref referencePeriodEncoding.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief One additional parameter of the reference period, as a GRIB2 scaled value.
///
struct AdditionalParameterForReferencePeriod {
    long scaleFactor;
    long scaledValue;
};

///
/// @brief The two additional parameters of the reference period (SOT percentiles).
///
struct AdditionalParametersForReferencePeriod {
    AdditionalParameterForReferencePeriod first;
    AdditionalParameterForReferencePeriod second;
};

///
/// @brief Resolve the additional parameters of the reference period from the MARS `quantile`.
///
/// @section Deduction contract
/// - Reads: `mars["quantile"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// Validation: `0 < low < high < denominator`, with `denominator = 100 * 10^k`.
///
/// @tparam MarsDict_t  Type of the MARS dictionary.
/// @tparam ParDict_t   Type of the parameter dictionary (unused).
/// @tparam OptDict_t   Type of the options dictionary (unused).
///
/// @param[in] mars  MARS dictionary providing `quantile` as a string.
/// @param[in] par   Parameter dictionary (unused).
/// @param[in] opt   Options dictionary (unused).
///
/// @return The first and second additional parameters (scale factor and scaled value each).
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If `quantile` is missing or malformed, or any unexpected error occurs during deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
AdditionalParametersForReferencePeriod resolve_AdditionalParametersForReferencePeriod_or_throw(const MarsDict_t& mars,
                                                                                               const ParDict_t& par,
                                                                                               const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {

        const auto quantile = get_or_throw<std::string>(mars, "quantile");

        const auto dash  = quantile.find('-');
        const auto colon = quantile.find(':');
        if (dash == std::string::npos || colon == std::string::npos || colon < dash) {
            throw Mars2GribDeductionException("Invalid SOT quantile `" + quantile + "`: expected `<low>-<high>:<den>`",
                                              Here());
        }

        const long low         = std::stol(quantile.substr(0, dash));
        const long high        = std::stol(quantile.substr(dash + 1, colon - dash - 1));
        const long denominator = std::stol(quantile.substr(colon + 1));

        long scale    = 0;
        long expected = 100;
        while (expected < denominator) {
            expected *= 10;
            ++scale;
        }
        if (expected != denominator || !(0 < low && low < high && high < denominator)) {
            throw Mars2GribDeductionException(
                "Invalid SOT quantile `" + quantile + "`: expected 0 < low < high < denominator = 100 * 10^k", Here());
        }

        const auto scaled = [scale](long value) {
            long factor = scale;
            while (factor > 0 && value % 10 == 0) {
                value /= 10;
                --factor;
            }
            return AdditionalParameterForReferencePeriod{factor, value};
        };

        const AdditionalParametersForReferencePeriod result =
            (2 * low >= denominator) ? AdditionalParametersForReferencePeriod{scaled(low), scaled(high)}
                                     : AdditionalParametersForReferencePeriod{scaled(high), scaled(low)};

        MARS2GRIB_LOG_RESOLVE([&]() {
            return "`additionalParametersForReferencePeriod` resolved from input dictionaries: quantile='" + quantile +
                   "' first=(scaleFactor=" + std::to_string(result.first.scaleFactor) +
                   ", scaledValue=" + std::to_string(result.first.scaledValue) +
                   ") second=(scaleFactor=" + std::to_string(result.second.scaleFactor) +
                   ", scaledValue=" + std::to_string(result.second.scaledValue) + ")";
        }());

        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `additionalParametersForReferencePeriod` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

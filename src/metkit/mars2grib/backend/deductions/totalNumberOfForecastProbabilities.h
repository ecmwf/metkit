// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file totalNumberOfForecastProbabilities.h
/// @brief Deduction of the GRIB `totalNumberOfForecastProbabilities` key.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **total number of forecast probabilities** of a probability product.
///
/// The value defaults to `1` and can be overridden by the user through the
/// parameter dictionary (`misc`) key `totalNumberOfForecastProbabilities`.
///
/// This deduction is independent from `forecastProbabilityNumber`
/// and performs no validation. The consistency between the two values is
/// checked by the `probability` concept where the keys are set.
///
/// Error handling follows a strict fail-fast strategy:
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - OVERRIDE: value taken from the parameter dictionary
/// - DEFAULT: default value used
///
/// @section References
/// Concept:
/// - @ref probabilityEncoding.h
///
/// Related deductions:
/// - @ref forecastProbabilityNumber.h
/// - @ref probabilityType.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

#include <string>

#include "eckit/log/Log.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `totalNumberOfForecastProbabilities` key.
///
/// @section Deduction contract
/// - Reads: `par["totalNumberOfForecastProbabilities"]`
/// - Writes: none
/// - Side effects: logging (OVERRIDE or DEFAULT)
/// - Failure mode: throws
///
/// @tparam MarsDict_t Type of the MARS dictionary (unused).
/// @tparam ParDict_t  Type of the parameter dictionary.
/// @tparam OptDict_t  Type of the options dictionary (unused).
///
/// @param[in] mars MARS dictionary (unused).
/// @param[in] par  Parameter dictionary, optionally providing `totalNumberOfForecastProbabilities`.
/// @param[in] opt  Options dictionary (unused).
///
/// @return The total number of forecast probabilities (`1` unless overridden).
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If any unexpected error occurs during deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TotalNumberOfForecastProbabilities_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                         const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        const auto userValue = get_opt<long>(par, "totalNumberOfForecastProbabilities");

        if (userValue.has_value()) {
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg =
                    "`totalNumberOfForecastProbabilities` overridden from parameter dictionary: value='";
                logMsg += std::to_string(*userValue);
                logMsg += "'";
                return logMsg;
            }());

            return *userValue;
        }

        constexpr long totalNumberOfForecastProbabilities = 1L;

        MARS2GRIB_LOG_DEFAULT([&]() {
            std::string logMsg = "`totalNumberOfForecastProbabilities` defaulted: value='";
            logMsg += std::to_string(totalNumberOfForecastProbabilities);
            logMsg += "'";
            return logMsg;
        }());

        return totalNumberOfForecastProbabilities;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `totalNumberOfForecastProbabilities` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

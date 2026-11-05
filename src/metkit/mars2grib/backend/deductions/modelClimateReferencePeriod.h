// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file modelClimateReferencePeriod.h
/// @brief Deduction of the model-climate time ranges of the GRIB reference period.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **time ranges of the model climate** used as reference period
/// (`typeOfStatisticalProcessingForTimeRangeForReferencePeriod`,
/// `indicatorOfUnitForTimeRangeForReferencePeriod`,
/// `lengthOfTimeRangeForReferencePeriod`).
///
/// The model climate is built from N reforecast years (outer time range, in
/// years), each sampled in a window of D days (inner time range, in days):
/// - outer: ModelClimate (code table 4.102), Year (code table 4.4),
///   length from `numberOfReforecastYearsInModelClimate`
/// - inner: ModelClimate (code table 4.102), Day (code table 4.4),
///   length from `numberOfDaysInClimateSamplingWindow`
///
/// Error handling follows a strict fail-fast strategy:
/// - missing mandatory keys cause immediate failure
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
/// Related deductions:
/// - @ref sampleSizeOfModelClimate.h
/// - @ref startOfReferencePeriod.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessingForTimeRangeForReferencePeriod.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief One time range of the reference period.
///
struct TimeRangeForReferencePeriod {
    long typeOfStatisticalProcessingForTimeRangeForReferencePeriod;
    long indicatorOfUnitForTimeRangeForReferencePeriod;
    long lengthOfTimeRangeForReferencePeriod;
};

///
/// @brief Model-climate time ranges of the reference period.
///
struct ModelClimateReferencePeriod {
    TimeRangeForReferencePeriod outer;
    TimeRangeForReferencePeriod inner;
};

///
/// @brief Resolve the model-climate time ranges of the reference period.
///
/// @section Deduction contract
/// - Reads: `par["numberOfReforecastYearsInModelClimate"]`, `par["numberOfDaysInClimateSamplingWindow"]`
///          (both mandatory)
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// @tparam MarsDict_t  Type of the MARS dictionary (unused).
/// @tparam ParDict_t   Type of the misc (parameter) dictionary providing the keys listed above.
/// @tparam OptDict_t   Type of the options dictionary (unused).
///
/// @param[in] mars  MARS dictionary (unused).
/// @param[in] par   Misc dictionary providing the keys listed above.
/// @param[in] opt   Options dictionary (unused).
///
/// @return The outer (years) and inner (days) time ranges of the model climate.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If a mandatory key is missing or any unexpected error occurs during deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
ModelClimateReferencePeriod resolve_ModelClimateReferencePeriod_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                         const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)mars;
    (void)opt;

    try {

        // Code table 4.102: Model climate
        constexpr long modelClimate =
            static_cast<long>(tables::TypeOfStatisticalProcessingForReferencePeriod::ModelClimate);

        const auto years = get_or_throw<long>(par, "numberOfReforecastYearsInModelClimate");
        const auto days  = get_or_throw<long>(par, "numberOfDaysInClimateSamplingWindow");

        // Code table 4.4: time units
        const ModelClimateReferencePeriod result{
            {modelClimate, static_cast<long>(tables::TimeUnit::Year), years},
            {modelClimate, static_cast<long>(tables::TimeUnit::Day), days},
        };

        MARS2GRIB_LOG_RESOLVE([&]() {
            return "`modelClimateReferencePeriod` resolved from input dictionaries: years='" + std::to_string(years) +
                   "' days='" + std::to_string(days) + "'";
        }());

        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `modelClimateReferencePeriod` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

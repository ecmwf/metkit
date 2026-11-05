// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file sampleSizeOfModelClimate.h
/// @brief Deduction of the sample size of the model climate.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **sample size of the model climate**, encoded in the GRIB key
/// `sampleSizeOfReferencePeriod`.
///
/// The value is not derivable from MARS alone. It must be supplied via the
/// misc (parameter) dictionary by the upstream tool.
///
/// Error handling follows a strict fail-fast strategy:
/// - a missing key causes immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref referencePeriodEncoding.h
///
/// Related deductions:
/// - @ref modelClimateReferencePeriod.h
/// - @ref startOfReferencePeriod.h
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
/// @brief Resolve the sample size of the model climate.
///
/// @section Deduction contract
/// - Reads: `par["sampleSizeOfModelClimate"]` (mandatory)
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// @tparam MarsDict_t  Type of the MARS dictionary (unused).
/// @tparam ParDict_t   Type of the misc (parameter) dictionary providing `sampleSizeOfModelClimate`.
/// @tparam OptDict_t   Type of the options dictionary (unused).
///
/// @param[in] mars  MARS dictionary (unused).
/// @param[in] par   Misc dictionary providing `sampleSizeOfModelClimate`.
/// @param[in] opt   Options dictionary (unused).
///
/// @return The sample size of the model climate.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key is missing, cannot be converted to `long`, or any unexpected error occurs during deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SampleSizeOfModelClimate_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)mars;
    (void)opt;

    try {

        const long sampleSizeOfModelClimate = get_or_throw<long>(par, "sampleSizeOfModelClimate");

        MARS2GRIB_LOG_RESOLVE([&]() {
            return "`sampleSizeOfModelClimate` resolved from input dictionaries: value='" +
                   std::to_string(sampleSizeOfModelClimate) + "'";
        }());

        return sampleSizeOfModelClimate;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `sampleSizeOfModelClimate` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

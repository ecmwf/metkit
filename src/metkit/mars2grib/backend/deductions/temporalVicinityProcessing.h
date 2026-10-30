// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file temporalVicinityProcessing.h
/// @brief Deduction of the GRIB `temporalVicinityProcessing` key.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **temporal vicinity processing** (GRIB2 Code Table 4.104) of a focal
/// statistics product.
///
/// The value is deduced exclusively from the MARS `param` (paramId).
/// No user override is supported.
///
/// Currently supported paramIds: 131089, 131090, 131091 (`Categorical`).
///
/// Error handling follows a strict fail-fast strategy:
/// - a paramId that is not handled by this deduction causes an immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved from one or more input dictionaries
///
/// @section References
/// Concept:
/// - @ref focalStatisticsEncoding.h
///
/// Related deductions:
/// - @ref spatialVicinityProcessing.h
/// - @ref temporalVicinityTowardsPastInHours.h
///
/// Related tables:
/// - @ref vicinityProcessing.h (tables)
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

#include <string>

#include "eckit/log/Log.h"
#include "metkit/mars2grib/backend/tables/vicinityProcessing.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `temporalVicinityProcessing` key.
///
/// @section Deduction contract
/// - Reads: `mars["param"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// @tparam MarsDict_t Type of the MARS dictionary. Must provide `param`.
/// @tparam ParDict_t  Type of the parameter dictionary (unused).
/// @tparam OptDict_t  Type of the options dictionary (unused).
///
/// @param[in] mars MARS dictionary providing the paramId.
/// @param[in] par  Parameter dictionary (unused).
/// @param[in] opt  Options dictionary (unused).
///
/// @return The temporal vicinity processing for the given paramId.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If `param` is missing or if the paramId is not handled by this deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::VicinityProcessing resolve_TemporalVicinityProcessing_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                       const OptDict_t& opt) {

    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        const auto param = get_or_throw<long>(mars, "param");

        tables::VicinityProcessing temporalVicinityProcessing = tables::VicinityProcessing::Missing;

        if (matchAny(param, 131089, 131090, 131091)) {
            temporalVicinityProcessing = tables::VicinityProcessing::Categorical;
        }
        else {
            throw Mars2GribDeductionException(
                "`temporalVicinityProcessing` is not defined for `param` " + std::to_string(param), Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`temporalVicinityProcessing` resolved from input dictionaries: value='";
            logMsg += tables::enum2name_VicinityProcessing_or_throw(temporalVicinityProcessing);
            logMsg += "'";
            return logMsg;
        }());

        return temporalVicinityProcessing;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `temporalVicinityProcessing` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

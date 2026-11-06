// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file typeOfRelationToReferenceDataset.h
/// @brief Deduction of the GRIB `typeOfRelationToReferenceDataset` key.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **type of relation to the reference dataset** (GRIB2 code table 4.101)
/// from the MARS dictionary.
///
/// Supported products:
/// - `type=efi|efic`: Extreme Forecast Index
/// - `type=sot`: Shift of Tails
/// - `type=ep` with `param` in [131020, 131025]: Anomaly
///
/// Any other product is not supported and causes a failure.
///
/// Error handling follows a strict fail-fast strategy:
/// - missing inputs or unsupported products cause immediate failure
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
/// - @ref typeOfReferenceDataset.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/tables/typeOfRelationToReferenceDataset.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the type of relation to the reference dataset (code table 4.101).
///
/// @section Deduction contract
/// - Reads: `mars["type"]`, `mars["param"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// @tparam MarsDict_t  Type of the MARS dictionary.
/// @tparam ParDict_t   Type of the parameter dictionary (unused).
/// @tparam OptDict_t   Type of the options dictionary (unused).
///
/// @param[in] mars  MARS dictionary providing `type` and `param`.
/// @param[in] par   Parameter dictionary (unused).
/// @param[in] opt   Options dictionary (unused).
///
/// @return The resolved `tables::TypeOfRelationToReferenceDataset`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If a key is missing, the product has no reference dataset logic implemented, or any unexpected error occurs.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::TypeOfRelationToReferenceDataset resolve_TypeOfRelationToReferenceDataset_or_throw(const MarsDict_t& mars,
                                                                                           const ParDict_t& par,
                                                                                           const OptDict_t& opt) {

    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {

        const auto marsType  = get_or_throw<std::string>(mars, "type");
        const auto marsParam = get_or_throw<long>(mars, "param");

        const auto resolved = [&]() {
            if (marsType == "efi" || marsType == "efic") {
                return tables::TypeOfRelationToReferenceDataset::ExtremeForecastIndex;
            }
            if (marsType == "sot") {
                return tables::TypeOfRelationToReferenceDataset::ShiftOfTails;
            }
            if (marsType == "ep" && matchAny(marsParam, range(131020, 131025))) {
                return tables::TypeOfRelationToReferenceDataset::Anomaly;
            }
            if (marsType == "ep" && matchAny(marsParam, range(133093, 133098))) {
                return tables::TypeOfRelationToReferenceDataset::StandardizedAnomaly;
            }
            throw Mars2GribDeductionException(
                "Logic not implemented for this product with Reference Period: type='" + marsType + "'", Here());
        };

        const auto result = resolved();

        MARS2GRIB_LOG_RESOLVE([&]() {
            return "`typeOfRelationToReferenceDataset` resolved from input dictionaries: value='" +
                   std::to_string(static_cast<long>(result)) + "'";
        }());

        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `typeOfRelationToReferenceDataset` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

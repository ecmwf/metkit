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
/// @file typeOfTimeIncrement.h
/// @brief Public deduction header for `typeOfTimeIncrement`.
///
/// Exposes `resolve_TypeOfTimeIncrement_or_throw`, the designated deduction
/// boundary for GRIB Code Table 4.11 resolution used by ProductTimeSpec.
///
/// The concrete deduction logic is intentionally still undefined. The current
/// implementation fails loudly so callers do not accidentally depend on
/// unspecified behavior while the boundary already exists in the architecture.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve `typeOfTimeIncrement`.
///
/// @section Deduction contract
///   - Reads (MARS): none yet
///   - Reads (par):  none yet
///   - Reads (opt):  none yet
///   - Writes:       none
///   - Side effects: none
///   - Failure mode: throws `Mars2GribDeductionException`
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary (reserved for future logic).
/// @param[in] par   Parameter dictionary (reserved for future logic).
/// @param[in] opt   Options dictionary (reserved for future logic).
///
/// @return The resolved GRIB `tables::TypeOfTimeIntervals` value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         always, because the deduction boundary exists but the semantic logic
///         has not been implemented yet.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::TypeOfTimeIntervals resolve_TypeOfTimeIncrement_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                 const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)mars;
    (void)par;
    (void)opt;

    throw Mars2GribDeductionException(
        "`typeOfTimeIncrement` deduction is not implemented yet; deduction boundary exists by design", Here());
}

}  // namespace metkit::mars2grib::backend::deductions

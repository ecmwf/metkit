/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file numberOfFrequencies.h
/// @brief Deduction of the GRIB `numberOfFrequencies` key.
///
/// Resolve the wave spectrum discretization size (`numberOfFrequencies`).
///
/// @section Deduction contract
/// - Reads: `par["numberOfFrequencies"]` (optional)
/// - Writes: none
/// - Side effects: logging (OVERRIDE or DEFAULT)
/// - Failure mode: throws on unexpected errors
///
/// If the parameter dictionary provides `numberOfFrequencies`, the value is
/// used verbatim. Otherwise a documented default (`54`) is applied.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `numberOfFrequencies` key.
///
/// @tparam MarsDict_t Type of the MARS dictionary (unused).
/// @tparam ParDict_t  Type of the parameter dictionary.
/// @tparam OptDict_t  Type of the options dictionary (unused).
///
/// @param[in] mars MARS dictionary (unused).
/// @param[in] par  Parameter dictionary; may contain `numberOfFrequencies`.
/// @param[in] opt  Options dictionary (unused).
///
/// @return Resolved number of frequencies.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If an unexpected error occurs while accessing the parameter dictionary.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_NumberOfFrequencies_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        if (has(par, "numberOfFrequencies")) {
            long numberOfFrequencies = get_or_throw<long>(par, "numberOfFrequencies");

            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`numberOfFrequencies` resolved from input dictionaries: value=";
                logMsg += std::to_string(numberOfFrequencies);
                return logMsg;
            }());

            return numberOfFrequencies;
        }
        else {
            long numberOfFrequencies = 54;

            MARS2GRIB_LOG_DEFAULT([&]() {
                std::string logMsg = "`numberOfFrequencies` resolved from input dictionaries: value=";
                logMsg += std::to_string(numberOfFrequencies);
                return logMsg;
            }());

            return numberOfFrequencies;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `numberOfFrequencies` from input dictionaries", Here()));
    };

    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

// Tables
#include "metkit/mars2grib/backend/tables/typeOfGeneratingProcess.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Optionally deduce the GRIB `typeOfGeneratingProcess` from MARS metadata.
 *
 * This deduction attempts to infer the GRIB *Type of Generating Process*
 * (GRIB2 Section 4, Code Table 3) based on a limited set of MARS keys.
 *
 * The deduction is **non-mandatory** and returns a value only when a
 * well-defined and explicitly supported mapping applies. If no supported
 * mapping is identified, the function returns `std::nullopt`.
 *
 * @important
 * This function is intentionally conservative. It must **not** guess or
 * approximate a generating process. If the deduction is not unambiguous,
 * it returns `std::nullopt` and leaves the responsibility to:
 * - a higher-level deduction, or
 * - an explicit default / missing value, or
 * - a hard failure (`or_throw`) depending on encoder policy.
 *
 * @section Current deduction logic
 * At present, the deduction applies only to the following case:
 *
 * - If `mars::type == "4i"`, the generating process is deduced as:
 *   `TypeOfGeneratingProcess::analysis_increment`
 *
 * All other cases result in `std::nullopt`.
 *
 * @section Inputs used
 * The following MARS keys are accessed:
 * - `type`
 * - `stream`
 * - `class`
 * - `param`
 *
 * Some of these keys are currently unused in the logic but are retrieved
 * explicitly to:
 * - document the intended deduction context
 * - ensure availability for future extensions
 * - fail early if required metadata is missing
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (currently unused)
 * @tparam OptDict_t  Type of the options dictionary (currently unused)
 *
 * @param[in] mars MARS dictionary providing metadata used for deduction
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return An optional `tables::TypeOfGeneratingProcess`:
 * - a value if the deduction applies
 * - `std::nullopt` if no supported deduction is identified
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - required MARS keys are missing
 *         - any unexpected error occurs while accessing the MARS dictionary
 *
 * @note
 * - This function does **not** rely on pre-existing GRIB header state.
 * - No defaulting or fallback is performed here.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: completeness][prio: medium]
 * - Extend the deduction logic to cover additional MARS combinations
 *   once formally specified.
 * - Re-evaluate whether this deduction should remain optional or be
 *   promoted to a mandatory `or_throw` deduction.
 * - Improve logging to document deduction decisions.
 *
 * @see tables::TypeOfGeneratingProcess
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<tables::TypeOfGeneratingProcess> resolve_TypeOfGeneratingProcess_opt(
    const MarsDict_t& mars, [[maybe_unused]] const ParDict_t& par, [[maybe_unused]] const OptDict_t& opt) {

    using metkit::mars2grib::backend::tables::TypeOfGeneratingProcess;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    // N.B. Sometimes this is overwritten by eccodes as a side effect of setting `param`
    try {

        // Get the mars records that are useful to deduce the typeOfGeneratingProcess
        std::string marsTypeVal = get_or_throw<std::string>(mars, "type");
        // std::string marsStreamVal = get_or_throw<std::string>(mars, "stream");
        // std::string marsClassVal = get_or_throw<std::string>(mars, "class");
        // long paramIdVal = get_or_throw<long>(mars, "param");

        // Deduce the typeOfGeneratingProcess
        if (marsTypeVal == "4i") {

            // Logging of the mars::model
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`typeOfGeneratingProcess`: mapped from `mars::type`";
                return logMsg;
            }());

            return {TypeOfGeneratingProcess::AnalysisIncrement};
        }
        else {

            // Logging of the mars::model
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`typeOfGeneratingProcess`: unable to map, return std::nullopt to skip deduction";
                return logMsg;
            }());

            return std::nullopt;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get entries from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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

#include <string>

#include "eckit/log/Log.h"


#include "metkit/mars2grib/backend/tables/productionStatusOfProcessedData.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `productionStatusOfProcessedData` key.
 *
 * This deduction determines the value of the GRIB
 * `productionStatusOfProcessedData` key based on the content of the
 * MARS request.
 *
 * The resolution logic is currently **minimal and conservative** and
 * reflects only officially agreed production semantics.
 *
 * ### Current behavior
 * - If `mars::class == "d1"`, the production status is set to
 *   `DestinationEarth`.
 *   This is **mandatory** for DestinE workflows, as ecCodes relies on
 *   this value to select the correct Local Use Section template.
 * - In all other cases, the value defaults to `Missing`.
 *
 * No inference is currently performed from `mars::type` or
 * `mars::stream`.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `productionStatusOfProcessedData`.
 * Any semantic validation, defaulting rules, or inference logic
 * **must be implemented here**.
 *
 * @warning
 * Returning `Missing` is a temporary and explicitly acknowledged
 * fallback. Relying on `Missing` as a long-term default is not
 * acceptable and must be resolved through a formal policy decision.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (currently unused)
 * @tparam OptDict_t  Type of the options dictionary (currently unused)
 *
 * @param[in] mars MARS dictionary; must contain the keys `class`, `type`,
 *                 and `stream`
 * @param[in] par  Parameter dictionary (unused in current logic)
 * @param[in] opt  Options dictionary (unused in current logic)
 *
 * @return The resolved `ProductionStatusOfProcessedData` enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - required MARS keys are missing
 *         - any unexpected error occurs during deduction
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: policy][prio: high]
 * - Define a formal mapping for `productionStatusOfProcessedData`
 *   based on combinations of:
 *     - `mars::class`
 *     - `mars::type`
 *     - `mars::stream`
 * - Align the default behavior with DGOV-approved production policies.
 * - Remove the unconditional fallback to `Missing` once policy
 *   decisions are finalized.
 *
 * @note
 * - This deduction does **not** rely on any pre-existing GRIB header
 *   state.
 * - The result is deterministic for a given MARS request.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::ProductionStatusOfProcessedData resolve_ProductionStatusOfProcessed_or_throw(const MarsDict_t& mars,
                                                                                     const ParDict_t& par,
                                                                                     const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get mars type from dictionary
        std::string marsClass  = get_or_throw<std::string>(mars, "class");
        std::string marsType   = get_or_throw<std::string>(mars, "type");
        std::string marsStream = get_or_throw<std::string>(mars, "stream");

        // TODO MIVAL: Need to check the value of `class` is valid in mars

        // Deduce typeOfProcessedData from mars type
        if (marsClass == "d1") {
            // This is mandatory for DestinE because it is used inside eccodes
            // to allocate the proper "localUseSection" template.
            // Setting this keyword reallocate the local use section.
            return tables::ProductionStatusOfProcessedData::DestinationEarth;
        }
        else {
            // TODO MIVAL: Here I set the default to missing, but need to clarify with DGOV team,
            // This need to be inferred from "type", "class", "stream"
            return tables::ProductionStatusOfProcessedData::Missing;
        };

        // Remove compiler warning
        __builtin_unreachable();
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to deduce `productionStatusOfProcessedData` from Mars or Parametrization dictionaries", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

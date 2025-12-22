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

// Tables
#include "metkit/mars2grib/backend/tables/backgroundProcess.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/enableOptions.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {


/**
 * @brief Resolve the GRIB `backgroundProcess` key from the MARS model identifier.
 *
 * This deduction resolves the value of the GRIB `backgroundProcess` key
 * by mapping the MARS model identifier (`mars::model`) to the corresponding
 * GRIB Background Process code.
 *
 * The mapping is explicit and strict: only a well-defined subset of
 * `mars::model` values is supported. Any unsupported or unknown value
 * results in a deduction error.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `backgroundProcess` in the encoder.
 * All semantic validation, mapping rules, and consistency checks
 * associated with this GRIB key **must be implemented here**.
 *
 * @section Source of truth
 * The authoritative definition of valid model identifiers and their
 * corresponding GRIB background process codes is maintained in:
 *
 *   `definitions/grib2/localConcepts/ecmf/modelNameConcept.def`
 *
 * Any changes to supported values **must** be reflected in that file
 * and mirrored here (preferably via automated code generation).
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (unused in this deduction)
 * @tparam OptDict_t  Type of the options dictionary (unused in this deduction)
 *
 * @param[in] mars MARS dictionary; must contain the key `model`
 * @param[in] par  Parameter dictionary (currently unused)
 * @param[in] opt  Options dictionary (currently unused)
 *
 * @return The resolved `BackgroundProcess` enumeration value corresponding
 *         to the provided `mars::model`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `model` is missing from the MARS dictionary
 *         - the `mars::model` value is not supported
 *         - mapping to a GRIB-compliant Background Process fails
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - The result is fully deterministic and reproducible.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: medium]
 * - Replace the hard-coded mapping with code generated directly from
 *   ecCodes GRIB code tables to guarantee long-term consistency.
 * - Validate mars::model value before using it for mapping.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::BackgroundProcess resolve_BackgroundProcess_or_throw(const MarsDict_t& mars,
                                                             [[maybe_unused]] const ParDict_t& par,
                                                             [[maybe_unused]] const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get backgroundProcess from mars dictionary
        std::string marsModelVal = get_or_throw<std::string>(mars, "model");

        // Logging of the mars::model
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`backgroundProcess`: mapped from `mars::model`: actual='";
            logMsg += marsModelVal + "'";
            return logMsg;
        }());

        // Return the mapped value if valid
        return tables::name2enum_BackgroundProcess_or_throw(marsModelVal);
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to resolve `backgroundProcess` from Mars and Par dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

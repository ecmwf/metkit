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

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `subCentre` identifier.
 *
 * This deduction resolves the value of the GRIB key `subCentre` using the
 * parameter dictionary.
 *
 * The value is retrieved from the parameter dictionary key `par::subCentre`.
 * If the key is not present, the deduction **defaults explicitly to `0`**,
 * which corresponds to the GRIB convention for an unspecified or default
 * sub-centre.
 *
 * @important
 * This function is the **single authoritative deduction** for the GRIB
 * `subCentre` key.
 * Any future validation, policy decision, or mapping against official
 * GRIB sub-centre tables **must be implemented here**.
 *
 * @section Semantics
 * - Input source: parameter dictionary (`par::subCentre`)
 * - Resolution type: mandatory
 * - Defaulting: explicit (`0`)
 * - Validation: numeric presence only
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; may contain `subCentre`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved GRIB `subCentre` value. If not explicitly provided,
 *         `0` is returned.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the parameter dictionary access fails
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - The default value `0` is applied intentionally and explicitly to ensure
 *   deterministic and reproducible encoding behavior.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: medium]
 * - Validate the returned value against official GRIB sub-centre code tables.
 * - Decide whether defaulting to `0` should remain unconditional or become
 *   policy-driven.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SubCentre_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get subCentre from par dictionary, defaulting to 0 if missing
        long subCentre = get_opt<long>(par, "subCentre").value_or(0L);

        // Logging of the centre deduction
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`subCentre`: mapped from `par::subCentre`: actual='";
            logMsg += std::to_string(subCentre) + "'";
            return logMsg;
        }());

        // Return validated origin
        return subCentre;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `subCentre` as string from Par dictionary", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

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
 * @brief Resolve the GRIB `centre` identifier from MARS metadata.
 *
 * This deduction resolves the value of the GRIB key `centre` by retrieving
 * the originating centre identifier from the MARS dictionary.
 *
 * The value is obtained directly from the MARS key `origin` and returned
 * verbatim. No normalization, translation, or defaulting is applied at
 * this stage.
 *
 * @important
 * This function is the **single authoritative deduction** for the GRIB
 * `centre` key.
 * Any future validation, normalization, or mapping to numeric GRIB centre
 * codes **must be implemented here**.
 *
 * @section Semantics
 * - Input source: MARS dictionary (`mars::origin`)
 * - Resolution type: mandatory
 * - Defaulting: none
 * - Validation: presence only (string semantics assumed)
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary; must contain the key `origin`
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The originating centre identifier as provided by MARS.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `origin` is missing from the MARS dictionary
 *         - the value cannot be retrieved as a string
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction does not rely on any pre-existing GRIB header state.
 * - The returned value is expected to be further interpreted or mapped
 *   by downstream encoding logic if a numeric GRIB centre code is required.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: medium]
 * - Introduce explicit validation or mapping against official GRIB centre
 *   code tables.
 * - Decide whether this deduction should return a numeric centre identifier
 *   instead of a string.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Centre_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Lookup origin from the mars dictionary
        auto origin = get_or_throw<std::string>(mars, "origin");

        // Logging of the mars::model
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`centre`: mapped from `mars::origin`: actual='";
            logMsg += origin + "'";
            return logMsg;
        }());

        // Return validated origin
        return origin;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `origin` as string from Mars dictionary", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

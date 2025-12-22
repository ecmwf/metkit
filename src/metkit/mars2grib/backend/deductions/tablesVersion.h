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
 * @brief Resolve the latest available GRIB tables version supported by ecCodes.
 *
 * This deduction determines the value of the GRIB
 * `tablesVersionLatest` key by querying the ecCodes library directly.
 *
 * The value is obtained by creating a GRIB2 sample handle and reading
 * the `tablesVersionLatest` key, which represents the most recent GRIB
 * tables version known to the ecCodes installation used at runtime.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `tablesVersionLatest` in the encoder.
 * The value returned reflects the ecCodes runtime environment and
 * therefore defines the maximum GRIB tables version that the encoder
 * may safely reference.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary (unused)
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary (unused)
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The latest GRIB tables version number supported by ecCodes.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - a GRIB2 sample cannot be created
 *         - the key `tablesVersionLatest` cannot be read
 *         - any unexpected ecCodes error occurs
 *
 * @note
 * - The returned value depends on the ecCodes version available at runtime.
 * - This deduction does not rely on any MARS or parameter input.
 * - The result is deterministic for a given ecCodes installation.
 *
 * @warning
 * Mixing GRIB messages produced with different ecCodes versions may lead
 * to inconsistent metadata if downstream consumers assume a fixed tables
 * version.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: robustness][prio: low]
 * - Cache the value to avoid repeated GRIB2 sample creation.
 * - Consider validating consistency between `tablesVersionLatest` and
 *   `tablesVersion` deductions.
 * - Optionally allow overriding this value for controlled regression tests.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TablesVersionLatest_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Extract the latest tables version from eccodes.
        long tablesVersionLatestVal = metkit::codes::codesHandleFromSample("GRIB2")->getLong("tablesVersionLatest");

        // Logging of the par::lengthOfTimeWindow
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "tablesVersion: looked up from eccodes sample as `tablesVersionLatest` with value: ";
            logMsg += std::to_string(tablesVersionLatestVal);
            return logMsg;
        }());

        return tablesVersionLatestVal;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Could not deduce `tablesVersion` from GRIB2 sample as `tablesVersionLatest`", Here()));
    }
};

/**
 * @brief Resolve a custom GRIB tables version from user parameters.
 *
 * This deduction resolves the GRIB `tablesVersion` key using an explicit
 * user-provided value supplied via the parameter dictionary (`par`).
 *
 * The value is read directly from the key `tablesVersion` and is assumed
 * to represent a GRIB tables version compatible with the ecCodes
 * installation used at runtime.
 *
 * @important
 * This function represents an **explicit user override** of the GRIB
 * tables version. It bypasses automatic detection mechanisms (e.g.
 * `tablesVersionLatest`) and therefore must be used with care.
 *
 * Incorrect or unsupported values may lead to invalid, non-portable,
 * or non-reproducible GRIB messages.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; must contain the key `tablesVersion`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The GRIB tables version explicitly specified by the user.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - the key `tablesVersion` is missing from the parameter dictionary
 *         - the value cannot be converted to a numeric GRIB tables version
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - No validation is performed against the ecCodes-supported tables
 *   available at runtime.
 * - Callers requiring strict reproducibility should validate this value
 *   against `tablesVersionLatest`.
 *
 * @warning
 * Using a custom tables version may cause silent divergence between
 * GRIB producers and consumers if software stacks evolve independently.
 *
 * @todo [owner: mds,dgov][scope: deduction][reason: robustness][prio: low]
 * - Add optional validation against `tablesVersionLatest`.
 * - Define clear precedence rules when both automatic and custom
 *   tables version deductions are available.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_TablesVersionCustom_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Extract the latest tables version from par dictionary.
        long tablesVersionCustomVal = get_or_throw<long>(par, "tablesVersion");

        // Logging of the par::lengthOfTimeWindow
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "tablesVersion: override from par dictionary with value: ";
            logMsg += std::to_string(tablesVersionCustomVal);
            return logMsg;
        }());

        return tablesVersionCustomVal;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Could not deduce `tablesVersion` from par dictionary", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

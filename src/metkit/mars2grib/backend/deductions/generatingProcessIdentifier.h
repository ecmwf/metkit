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

#include <optional>
#include <string>


#include "eckit/log/Log.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/enableOptions.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Optionally resolve the GRIB `generatingProcessIdentifier` key from parameters.
///
/// This deduction provides an **optional passthrough resolution** for the GRIB
/// `generatingProcessIdentifier` key.
///
/// When present, the value is read **verbatim** from the parameter dictionary
/// (`par`) and returned without modification or validation.
/// If the key is not present, the function returns `std::nullopt`.
///
/// @important
/// This function performs **no deduction logic** and **no semantic validation**.
/// It exists solely to allow expert or legacy workflows to explicitly inject
/// a GRIB `generatingProcessIdentifier` value via the parameter dictionary.
///
/// The use of this mechanism is **discouraged** for production workflows, as it
/// may lead to inconsistent or non-reproducible GRIB headers if not coordinated
/// with the rest of the encoding logic.
///
/// @section Semantics
/// - Input source: parameter dictionary (`par`)
/// - Resolution type: optional passthrough
/// - Validation: none
/// - Defaulting: none
///
/// @tparam MarsDict_t Type of the MARS dictionary (unused)
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary (unused)
///
/// @param[in] mars MARS dictionary (unused)
/// @param[in] par  Parameter dictionary; may contain `generatingProcessIdentifier`
/// @param[in] opt  Options dictionary (unused)
///
/// @return An optional `long`:
/// - the value of `generatingProcessIdentifier` if present in `par`
/// - `std::nullopt` otherwise
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If any unexpected error occurs while accessing the parameter dictionary.
///
/// @warning
/// This deduction must **not** be relied upon as the primary mechanism for setting
/// `generatingProcessIdentifier`.
/// A proper, deterministic deduction based on MARS semantics or encoder policy
/// should be preferred whenever possible.
///
/// @todo [owner: mds,dgov][scope: deduction][reason: legacy][prio: medium]
/// - Need to define a proper table and a proper logic to deduce the `generatingProcessIdentifier`
/// - Evaluate whether this passthrough deduction can be removed once all
/// generating process identifiers are derived deterministically.
/// - Consider replacing this with a validated, table-driven deduction.
///
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_GeneratingProcessIdentifier_opt([[maybe_unused]] const MarsDict_t& mars,
                                                            const ParDict_t& par,
                                                            [[maybe_unused]] const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get generatingProcessIdentifier from mars dictionary
        std::optional<long> generatingProcessIdentifierVal = get_opt<long>(par, "generatingProcessIdentifier");

        if (generatingProcessIdentifierVal.has_value()) {

            // Logging of the par::generatingProcessIdentifier
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg =
                    "`generatingProcessIdentifier`: mapped from `par::generatingProcessIdentifier`: actual='";
                logMsg += std::to_string(generatingProcessIdentifierVal.value()) + "'";
                return logMsg;
            }());

            // Return the generatingProcessIdentifier from the parameter dictionary
            return {generatingProcessIdentifierVal};
        }
        else {

            // Logging of the par::generatingProcessIdentifier absence
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg =
                    "`generatingProcessIdentifier`: `par::generatingProcessIdentifier` not present, return "
                    "std::nullopt to skip deduction";
                return logMsg;
            }());

            // Key not present; return std::nullopt
            return std::nullopt;
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `generatingProcessIdentifier` from parameter dictionary", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

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
/// @file generatingProcessIdentifier.h
/// @brief Optional deduction of the GRIB `generatingProcessIdentifier` key.
///
/// This header provides an optional passthrough deduction for the GRIB
/// `generatingProcessIdentifier` key.
///
/// @section Deduction contract
/// - Reads: `par["generatingProcessIdentifier"]` (optional)
/// - Writes: none
/// - Side effects: logging (RESOLVE when present, otherwise DEFAULT-style skip message)
/// - Failure mode: returns `std::nullopt` when absent; throws on unexpected errors
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

#include <optional>
#include <string>


#include "eckit/log/Log.h"
#include "metkit/mars2grib/backend/tables/backgroundProcess.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/config/LibMetkit.h"
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

///
/// @brief Model version code of `generatingProcessIdentifier` in the numbering of the component scheme.
///
/// The legacy IFS numbering (ecCodes `modelVersionConcept.IFS.255.def`: cy49r2 = 159, cy50r1 = 161,
/// cy50r2 = 162) is what the input carries (the GRIB1 header, or the `ncycle` override). The component
/// scheme of ECC-2354 numbers the cycles from cy49r2 = 0 (`modelVersionConcept.<model>.<code>.def`: cy50r1
/// = 1 as in ldas, cy50r2 = 2), the same for every component. A value below 100 is already in the new
/// numbering; an unknown legacy value is kept, and the model version then resolves to "unknown".
///
inline long translate_GeneratingProcessIdentifier(long value, tables::BackgroundProcess backgroundProcess) {
    if (!tables::isComponentScheme(backgroundProcess) || value < 100) {
        return value;
    }
    switch (value) {
        case 159:
            return 0;  // cy49r2
        case 161:
            return 1;  // cy50r1
        case 162:
            return 2;  // cy50r2
        default:
            MARS2GRIB_LOG_RESOLVE([&]() {
                return "`generatingProcessIdentifier`: legacy value " + std::to_string(value) +
                       " has no equivalent in the component scheme, kept";
            }());
            return value;
    }
}

}  // namespace metkit::mars2grib::backend::deductions

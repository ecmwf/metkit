// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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

namespace {


long defaultGeneratingProcessIdentifier(long backgroundProcess) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    switch (backgroundProcess) {
        case 1:        // aifs-single
            return 6;  // aifs-single-mse-v3.0

        case 2:        // aifs-ens
            return 3;  // aifs-ens-crps-v3

        case 3:        // aifs-single-mse
            return 6;  // v3.0

        case 4:        // aifs-ens-crps
            return 3;  // v3

        case 5:        // aifs-ens-diff
            return 1;  // v0.1

        case 6:        // aifs-compo-single
            return 2;  // aifs-compo-single-mse-v0.2

        case 7:        // aifs-compo-ens
            return 1;  // aifs-compo-ens-crps-v0.1

        case 8:        // aifs-compo-single-mse
            return 2;  // v0.2

        case 9:        // aifs-compo-ens-crps
            return 1;  // v0.01

        case 10:       // aifs-subs
            return 1;  // aifs-subs-crps-v0.1

        case 11:       // aifs-subs-crps
            return 1;  // v0.1

        case 144:      // gfas
            return 3;  // v1.4.2

        case 246:      // ldas
        case 248:      // ecwam
        case 250:      // nemo
        case 252:      // ifs-compo
        case 254:      // ifs
            return 2;  // cy50r2
    }

    throw Mars2GribDeductionException(
        "No default generatingProcessIdentifier exists for backgroundProcess " + std::to_string(backgroundProcess),
        Here());
}

}  // namespace

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
long resolve_GeneratingProcessIdentifier_opt([[maybe_unused]] const MarsDict_t& mars, const ParDict_t& par,
                                             [[maybe_unused]] const OptDict_t& opt, long backgroundProcess) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        if (has(par, "generatingProcessIdentifier")) {
            const long generatingProcessIdentifier = get_or_throw<long>(par, "generatingProcessIdentifier");

            MARS2GRIB_LOG_RESOLVE([&]() {
                return "`generatingProcessIdentifier`: mapped from `par::generatingProcessIdentifier`: actual='" +
                       std::to_string(generatingProcessIdentifier) + "'";
            }());

            return generatingProcessIdentifier;
        }
        else {
            const long generatingProcessIdentifier = defaultGeneratingProcessIdentifier(backgroundProcess);

            MARS2GRIB_LOG_RESOLVE([&]() {
                return "`generatingProcessIdentifier`: defaulted to " + std::to_string(generatingProcessIdentifier) +
                       " for backgroundProcess " + std::to_string(backgroundProcess);
            }());

            return generatingProcessIdentifier;
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

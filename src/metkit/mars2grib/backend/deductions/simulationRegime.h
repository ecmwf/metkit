/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file simulationRegime.h
/// @brief Public deduction header for the normalized ProductTimeSpec simulation regime.
///
/// Exposes `resolve_SimulationRegime_or_throw`, the canonical entry point that
/// derives the normalized ProductTimeSpec simulation regime from the raw MARS
/// `class` key.
///
/// This deduction owns:
/// - direct `class` dictionary access;
/// - the ProductTimeSpec-specific mapping from MARS class to simulation regime.
///
/// This deduction does NOT:
/// - classify simulation type;
/// - construct ProductTimeSpec model artifacts;
/// - validate unrelated product policies.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the normalized ProductTimeSpec simulation regime.
///
/// @section Deduction contract
///   - Reads (MARS): `class`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - MARS class `ml` -> `SimulationRegime::AIFS`;
/// - any other present MARS class -> `SimulationRegime::IFS`;
/// - missing or malformed `class` -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `class`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return The normalized ProductTimeSpec simulation regime.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on missing, malformed, or unsupported raw `class` input, with the
///         original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
SimulationRegime resolve_SimulationRegime_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        const std::string klass       = get_or_throw<std::string>(mars, "class");
        const SimulationRegime result = klass == "ml" ? SimulationRegime::AIFS : SimulationRegime::IFS;

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`simulationRegime` resolved from input dictionaries: value='"} +
                   (result == SimulationRegime::AIFS ? "AIFS" : "IFS") + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `simulationRegime` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

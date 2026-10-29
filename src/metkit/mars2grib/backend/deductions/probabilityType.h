// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file probabilityType.h
/// @brief Deduction of the GRIB `probabilityType` key.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **probability type** (GRIB2 Code Table 4.9) of a probability product.
///
/// The value is deduced exclusively from the MARS `param` (paramId), mirroring
/// the definitions in ecCodes `grib2/paramId.def`. No user override is supported.
///
/// Error handling follows a strict fail-fast strategy:
/// - a paramId that is not handled by this deduction causes an immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved from one or more input dictionaries
///
/// @section References
/// Concept:
/// - @ref probabilityEncoding.h
///
/// Related tables:
/// - @ref probabilityType.h (tables)
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

#include <string>

#include "eckit/log/Log.h"
#include "metkit/mars2grib/backend/tables/probabilityType.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `probabilityType` key.
///
/// @section Deduction contract
/// - Reads: `mars["param"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// The mapping follows ecCodes `grib2/paramId.def`:
/// - `AboveLowerLimit` (3): 131001-131003, 131006-131010, 131021, 131024-131025,
///   131060-131063, 131066-131071, 131074-131077, 131082-131088, 131098-131100,
///   131256-131259, 133093-133095
/// - `BelowLowerLimit` (0): 131020, 131022, 131023, 131064, 131065, 131073, 133096-133098
/// - `EqualToLowerLimit` (5): 131004, 131005
/// - `CountsOfCategoricalBoolean` (9): 131089, 131090, 131091
///
/// @tparam MarsDict_t Type of the MARS dictionary. Must provide `param`.
/// @tparam ParDict_t  Type of the parameter dictionary (unused).
/// @tparam OptDict_t  Type of the options dictionary (unused).
///
/// @param[in] mars MARS dictionary providing the paramId.
/// @param[in] par  Parameter dictionary (unused).
/// @param[in] opt  Options dictionary (unused).
///
/// @return The probability type for the given paramId.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If `param` is missing or if the paramId is not handled by this deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::ProbabilityType resolve_ProbabilityType_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                         const OptDict_t& opt) {

    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        const auto param = get_or_throw<long>(mars, "param");

        tables::ProbabilityType probabilityType = tables::ProbabilityType::Missing;

        if (matchAny(param, 131089, 131090, 131091)) {
            probabilityType = tables::ProbabilityType::CountsOfCategoricalBoolean;
        }
        else if (matchAny(param, 131004, 131005)) {
            probabilityType = tables::ProbabilityType::EqualToLowerLimit;
        }
        else if (matchAny(param, 131020, 131022, 131023, 131064, 131065, 131073, range(133096, 133098))) {
            probabilityType = tables::ProbabilityType::BelowLowerLimit;
        }
        else if (matchAny(param, range(131001, 131003), range(131006, 131010), 131021, 131024, 131025,
                          range(131060, 131063), range(131066, 131071), range(131074, 131077), range(131082, 131088),
                          range(131098, 131100), range(131256, 131259), range(133093, 133095))) {
            probabilityType = tables::ProbabilityType::AboveLowerLimit;
        }
        else {
            throw Mars2GribDeductionException("`probabilityType` is not defined for `param` " + std::to_string(param),
                                              Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`probabilityType` resolved from input dictionaries: value='";
            logMsg += tables::enum2name_ProbabilityType_or_throw(probabilityType);
            logMsg += "'";
            return logMsg;
        }());

        return probabilityType;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `probabilityType` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

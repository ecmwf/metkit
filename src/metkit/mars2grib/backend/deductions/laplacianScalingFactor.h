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
/// @file laplacianOperator.h
/// @brief Deduction of a fixed laplacian operator for spectral complex packing.
///
/// Exposes `resolve_FixedLaplacianOperator_or_throw`, which decides whether a
/// spectral field must be encoded with a fixed laplacian operator instead of
/// the one fitted by ecCodes from the field values.
///
/// This reproduces the behaviour of IFS (`GRIB_SET_PACKING` in
/// `arpifs/module/grib_utils_mod.F90`, with `LFORCEZ` and `LFORCELNSP`
/// enabled by default in `sugrib.F90`), which encodes geopotential (`z`) and
/// logarithm of surface pressure (`lnsp`) on model levels with a laplacian
/// operator of `0.5`.
///
/// This deduction owns:
/// - direct `levtype` dictionary access;
/// - the mapping from (`paramId`, `levtype`) to a fixed laplacian operator.
///
/// This deduction does NOT:
/// - compute the laplacian operator from the field values (done by ecCodes);
/// - check that the field is spectral (done by the calling concept).
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/paramId.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve a fixed laplacian operator for spectral complex packing.
///
/// @section Deduction contract
///   - Reads (MARS): `param`, `levtype`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `paramId` 129 (`z`) or 152 (`lnsp`) on `levtype=ml` -> `0.5`;
/// - anything else -> no fixed value (ecCodes computes the operator).
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `param` and `levtype`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return The fixed laplacian operator, or `std::nullopt` if ecCodes must
///         compute it from the field values.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on missing or malformed `param` or `levtype`, with the original
///         cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<double> resolve_FixedLaplacianOperator_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                              const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Extract required fields from the MARS dictionary.
        const long paramId        = get_or_throw<long>(mars, "param");
        const std::string levtype = get_or_throw<std::string>(mars, "levtype");

        // IFS: LFORCEZ and LFORCELNSP in GRIB_SET_PACKING (arpifs/module/grib_utils_mod.F90)
        const bool isFixed                 = (paramId == 129 || paramId == 152) && levtype == "ml";
        const std::optional<double> result = isFixed ? std::optional<double>{0.5} : std::nullopt;

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`laplacianOperator` resolved from input dictionaries: value='"} +
                   (result ? std::to_string(*result) : std::string{"computed"}) + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `laplacianOperator` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
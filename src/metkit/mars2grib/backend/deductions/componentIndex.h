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
/// @file componentIndex.h
/// @brief Deduction of the model-error realization identifier.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **model-error realization identifier** (`componentIndex`)
/// from MARS metadata.
///
/// The deduction retrieves the realization identifier explicitly from the
/// MARS dictionary and returns it verbatim, without applying inference,
/// defaulting, or semantic interpretation.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - enforcing deterministic resolution rules
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply heuristic or data-driven inference
/// - do NOT validate against GRIB code tables unless explicitly required
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or invalid inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved directly from input dictionaries
///
/// @section References
/// Concept:
/// - @ref modelErrorEncoding.h
///
/// Related deductions:
/// - @ref numberOfComponents.h
/// - @ref modelErrorType.h
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <string>

#include "eckit/log/Log.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the model-error realization identifier.
///
/// @section Deduction contract
/// - Reads: `mars["type"]`, `mars["number"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// This deduction retrieves the model-error realization identifier from
/// the MARS dictionary. For requests with `type=eme`, the MARS key
/// `number` identifies the realization within the model-error ensemble
/// (not an ensemble-forecast member).
///
/// The value is treated as mandatory and is returned verbatim as a
/// numeric identifier. No inference, defaulting, or validation against
/// GRIB code tables is performed.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must provide the key `number`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused).
///
/// @param[in] mars
/// MARS dictionary from which the realization identifier is retrieved.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The model-error realization identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If:
/// - the key `type` is missing or not equal to `"eme"` (defence-in-depth:
///   `componentIndex` is only meaningful for model-error products),
/// - the key `number` is missing, cannot be converted to `long`,
/// - or any unexpected error occurs during deduction.
///
/// @note
/// This deduction assumes that the realization identifier is explicitly
/// provided by the MARS dictionary and does not attempt any semantic
/// interpretation or consistency checking.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_ComponentIndex_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Defence-in-depth: componentIndex is only meaningful for type=eme
        // (model-error products). Resolving it for any other request type
        // indicates a serious upstream contract violation (wrong recipe,
        // matcher bypass, etc.) and must be surfaced as a hard failure
        // with an unambiguous diagnostic.
        const std::string typeVal = get_or_throw<std::string>(mars, "type");
        if (typeVal != "eme") {
            throw Mars2GribDeductionException(std::string("`componentIndex` requested for a non-`eme` request: "
                                                          "`mars[\"type\"]` is `") +
                                                  typeVal +
                                                  "` but only `eme` is supported. This is a serious upstream "
                                                  "contract violation: the model-error deduction was reached "
                                                  "for a request that is not a model-error product. Check "
                                                  "recipe selection and matcher dispatch.",
                                              Here());
        }

        // Retrieve mandatory MARS number (model-error realization id)
        long componentIndex = get_or_throw<long>(mars, "number");

        // Emit RESOLVE log entry
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`componentIndex` resolved from input dictionaries: value=";
            logMsg += std::to_string(componentIndex);
            return logMsg;
        }());

        // Success exit point
        return componentIndex;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `componentIndex` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

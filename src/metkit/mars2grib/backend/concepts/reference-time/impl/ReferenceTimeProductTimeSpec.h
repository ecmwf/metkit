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
/// @file impl/ReferenceTimeProductTimeSpec.h
/// @brief Reference-time-specific transport struct built from
///        `ProductTimeAnchorSpec`.
///
/// Exposes `ReferenceTimeProductTimeSpec` plus the pure builder
/// `build_ReferenceTimeProductTimeSpec_or_throw`, which turns one final
/// immutable `backend::models::product_time_spec::ProductTimeAnchorSpec` into
/// the reference-time-facing transport struct consumed by the reference-time
/// concept.
///
/// This header owns only the reference-time-facing transport type and the
/// builder. It does NOT perform GRIB encoding itself.
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

#include <exception>

#include "eckit/types/DateTime.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_::impl {

///
/// @brief Reference-time-facing transport struct derived from
///        `ProductTimeAnchorSpec`.
///
/// The struct contains the resolved anchor datetimes needed by the
/// reference-time concept. Every member is copied directly from the final
/// immutable ProductTimeSpec anchor artifact after any extractor-level
/// validation succeeds.
///
struct ReferenceTimeProductTimeSpec {
    /// @brief Resolved anchor label datetime.
    eckit::DateTime labelDateTime{};

    /// @brief Resolved anchor initial-conditions datetime.
    eckit::DateTime initialConditionsDateTime{};

    /// @brief Resolved anchor reference datetime.
    eckit::DateTime referenceDateTime{};
};

///
/// @brief Build the reference-time transport struct from one final
///        `ProductTimeAnchorSpec`.
///
/// The builder reads only the final immutable backend-model
/// `ProductTimeAnchorSpec` and materializes the reference-time-facing
/// representation consumed by the reference-time concept.
///
/// Build rules:
/// - `referenceDateTime` must equal `initialConditionsDateTime`;
/// - the three anchor datetimes are then copied directly from the resolved
///   anchor artifact.
///
/// @param[in] spec Final immutable backend-model `ProductTimeAnchorSpec`.
/// @return Fully populated `ReferenceTimeProductTimeSpec`.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribGenericException on
///         invalid anchor semantics or any unexpected failure, with the
///         original cause preserved through nested exceptions.
///
inline ReferenceTimeProductTimeSpec build_ReferenceTimeProductTimeSpec_or_throw(
    const models::product_time_spec::ProductTimeAnchorSpec& spec) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const auto& anchor = spec.anchor();

        if (anchor.referenceDateTime != anchor.initialConditionsDateTime) {
            throw Mars2GribGenericException(
                "`ReferenceTimeProductTimeSpec` requires `referenceDateTime` to equal `initialConditionsDateTime`",
                Here());
        }

        ReferenceTimeProductTimeSpec out;
        out.labelDateTime             = anchor.labelDateTime;
        out.initialConditionsDateTime = anchor.initialConditionsDateTime;
        out.referenceDateTime         = anchor.referenceDateTime;
        return out;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to build `ReferenceTimeProductTimeSpec` from `ProductTimeAnchorSpec`", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_::impl

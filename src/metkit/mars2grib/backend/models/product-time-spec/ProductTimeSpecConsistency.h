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
/// @file ProductTimeSpecConsistency.h
/// @brief Final consistency validation surface for ProductTimeSpec assembly.
///
/// Exposes `validate_ProductTimeSpecConsistency_or_throw(...)`, the final model
/// stage that validates the already-built ProductTimeSpec artifacts together.
///
/// This header intentionally stays small. It owns only the public final
/// consistency entry point and the normative documentation of the remaining
/// whole-object checks that require all previously built artifacts. Internal
/// helper logic lives in `detail/productTimeSpecConsistency_details.h`.
///
/// The final consistency stage validates relationships that are meaningful only
/// once the complete set of resolved artifacts already exists:
///
/// | Concern | Inputs involved | Check |
/// |---------|-----------------|-------|
/// | anchor consistency | `anchorType`, `anchor` | classification and artifact agree; datetime ordering holds |
/// | domain consistency | `shapeType`, `anchor`, `domain` | support ordering holds; from-start begins at `referenceDateTime` |
/// | window cardinality | `shapeType`, `windows`, `input.stattype` | shape-specific canonical window count is correct |
/// | support placement | `domain`, `windows` | outermost window range reproduces `domainStartDateTime` |
/// | increment consistency | `incrementType`, `windows` | sentinel vs real increment semantics agree |
/// | fake-single-loop/double-loop consistency | `shapeType`, `windows`, `input` | 2 equal windows, outer `IndexProcessing`, inner caller processing |
///
/// This stage does not rebuild domain or windows. It consumes only artifacts and
/// classifications already produced by earlier model stages.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecWindows.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecConsistency_details.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {

template <class Input_t>
void validate_ProductTimeSpecConsistency_or_throw(const Input_t& input,
                                                  TimeAnchorKind anchorType,
                                                  ProductTimeSpecShapeKind shapeType,
                                                  TimeIncrementKind incrementType,
                                                  const ProductTimeSpecAnchor& anchor,
                                                  const ProductTimeSpecDomain& domain,
                                                  const ProductTimeSpecWindows& windows) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        detail::checkProductTimeSpecConsistencyAnchor_or_throw(input, anchorType, anchor);
        detail::checkProductTimeSpecConsistencyDomainOrdering_or_throw(input, domain);
        detail::checkProductTimeSpecConsistencyWindowCardinality_or_throw(input, shapeType, windows);

        if (shapeType == ProductTimeSpecShapeKind::Instant) {
            detail::checkProductTimeSpecConsistencyInstant_or_throw(
                input,
                incrementType,
                domain,
                windows);
            return;
        }

        detail::checkProductTimeSpecConsistencyOutermostSupport_or_throw(
            input,
            anchor,
            domain,
            windows);

        if (shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop) {
            detail::checkProductTimeSpecConsistencyFromStart_or_throw(
                input,
                anchor,
                domain);
        }

        if (incrementType == TimeIncrementKind::AifsPureMissingIncrement) {
            detail::checkProductTimeSpecConsistencyAifsMissingIncrement_or_throw(
                input,
                shapeType,
                windows);
        }

        if (shapeType == ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop) {
            detail::checkProductTimeSpecConsistencyFakeSingleLoopDoubleLoop_or_throw(
                input,
                windows);
        }
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to validate `ProductTimeSpec` final consistency from normalized input",
            input.to_json(),
            Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models

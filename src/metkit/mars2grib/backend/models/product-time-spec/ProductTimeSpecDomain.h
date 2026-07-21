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
/// @file ProductTimeSpecDomain.h
/// @brief Public domain-build surface for ProductTimeSpec.
///
/// Exposes the domain-build public model API:
/// - `ProductTimeSpecDomain`, the resolved absolute support domain artifact;
/// - `build_ProductTimeSpecDomain_or_throw(...)`.
///
/// This header intentionally stays small. It owns only the public domain build
/// artifact, the public build entry point, and the normative documentation of
/// support placement. Internal helper logic lives in
/// `detail/productTimeSpecDomain_details.h`, while shared time arithmetic lives
/// in `detail/ProductTimeSpecTimeUtils.h`.
///
/// Domain construction resolves the absolute support interval of one product.
/// The build always starts from the already-resolved anchor and the previously
/// classified shape. The support end is always built from
/// `referenceDateTime + step`, while the support start is determined by the
/// outermost range implied by the chosen shape.
///
/// The placement rules are:
///
/// | `ProductTimeSpecShapeKind` | `domainEndDateTime` source | outer range used for placement | `domainStartDateTime` rule                |
/// |----------------------------|----------------------------|--------------------------------|-------------------------------------------|
/// | `Instant`                  | `referenceDateTime + step` | none                           | `domainStartDateTime = domainEndDateTime` |
/// | `StandardSingleLoop`       | `referenceDateTime + step` | `timespan.duration`            | `domainEndDateTime - outerRange`          |
/// | `MultiLoop`                | `referenceDateTime + step` | first parsed `stattype` range  | `domainEndDateTime - outerRange`          |
/// | `FakeDoubleLoopSingleLoop` | `referenceDateTime + step` | first parsed `stattype` range  | `domainEndDateTime - outerRange`          |
/// | `FromStartSingleLoop`      | `referenceDateTime + step` | resolved `step`                | `domainStartDateTime = referenceDateTime` |
/// | `FakeSingleLoopDoubleLoop` | `referenceDateTime + step` | `timespan.duration`            | `domainEndDateTime - outerRange`          |
///
/// Calendar-aligned outer ranges impose additional placement rules:
/// - day-based outer ranges require `domainEndDateTime` at midnight;
/// - month-based outer ranges require `domainEndDateTime` on day one at
///   midnight.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "eckit/types/DateTime.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecDomain_details.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDataTypes.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {


///
/// @brief Build the absolute support domain from input, classifications, and anchor.
///
/// The domain build stage receives the normalized input snapshot, all resolved
/// classifications, and the already-built anchor artifact. The current placement
/// rules depend primarily on the resolved shape and anchor, but the complete set
/// of classifications is accepted as part of the uniform ProductTimeSpec build
/// pipeline.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] anchorType Previously resolved anchor classification.
/// @param[in] shapeType Previously resolved shape classification.
/// @param[in] incrementType Previously resolved time-increment classification.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @return Fully resolved `ProductTimeSpecDomain` artifact.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         placement or validation failure, with `input.to_json()` attached as
///         context.
///
template <class Input_t>
ProductTimeSpecDomain build_ProductTimeSpecDomain_or_throw(const Input_t& input,
                                                           TimeAnchorKind anchorType,
                                                           ProductTimeSpecShapeKind shapeType,
                                                           TimeIncrementKind incrementType,
                                                           const ProductTimeSpecAnchor& anchor) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        detail::checkProductTimeSpecDomainAnchorConsistency_or_throw(
            input,
            anchorType,
            anchor);

        const eckit::DateTime domainEndDateTime =
            detail::buildProductTimeSpecDomainEndDateTime_or_throw(input, anchor);
        const deductions::TimeDuration outermostRange =
            detail::buildProductTimeSpecDomainOutermostRange_or_throw(input, shapeType);

        detail::checkProductTimeSpecDomainOutermostAlignment_or_throw(
            input,
            domainEndDateTime,
            outermostRange);

        const eckit::DateTime domainStartDateTime =
            detail::buildProductTimeSpecDomainStartDateTime_or_throw(
                input,
                shapeType,
                anchor,
                domainEndDateTime,
                outermostRange);

        detail::checkProductTimeSpecDomainConsistency_or_throw(
            input,
            shapeType,
            anchor,
            domainStartDateTime,
            domainEndDateTime,
            outermostRange);

        (void)incrementType;

        return ProductTimeSpecDomain{domainStartDateTime, domainEndDateTime};
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to build `ProductTimeSpecDomain` from normalized input",
            input.to_json(),
            Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models

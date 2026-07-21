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
/// @file ProductTimeSpecShapeClassification.h
/// @brief Public shape-classification surface for ProductTimeSpec.
///
/// Exposes the shape-classification public model API:
/// - `ProductTimeSpecShapeKind`, the shape classification enum;
/// - `classify_ProductTimeSpecShape_or_throw(...)`.
///
/// This header owns only the public shape-classification type, the public
/// classification entry point, and the normative documentation of the shape
/// classification model. Internal helper logic lives in
/// `detail/productTimeSpecShapeClassification_details.h`.
///
/// Shape classification determines the structural temporal representation used
/// later by ProductTimeSpec window and domain construction. It depends on the
/// normalized `timespan` representation, the parsed `stattype` cardinality,
/// MARS contextual policy keyed by `(class, stream)` and `(type, class,
/// paramId)`, and the local consistency rules involving `step` and `type`.
///
/// The primary classification table is:
///
/// | `timespan` state | `stattype` blocks | Extra condition                                                                         | `ProductTimeSpecShapeKind` |
/// |------------------|-------------------|-----------------------------------------------------------------------------------------|----------------------------|
/// | missing          | `0`               | `allowMissingTimespanForInstantProduct == true`                                         | `Instant`                  |
/// | `none`           | `0`               | none                                                                                    | `Instant`                  |
/// | `none`           | `1`               | fake-double-loop representation required for `(class, stream)`                          | `FakeDoubleLoopSingleLoop` |
/// | duration         | `0`               | identified index-statistics `(type, class, paramId)` policy                             | `FakeSingleLoopDoubleLoop` |
/// | duration         | `0`               | fake-single-loop/double-loop policy does not match and fake-double-loop is not required | `StandardSingleLoop`       |
/// | duration         | `>= 1`            | none                                                                                    | `MultiLoop`                |
/// | from-start       | `0`               | if resolved `step == 0`, `allowZeroLengthFsWindow == true`, tsp == accumulation         | `FromStartSingleLoop`      |
///
/// The classifier also enforces the following local structural rejection rules:
///
/// | Condition                                                                                 | Outcome |
/// |-------------------------------------------------------------------------------------------|---------|
/// | `type == "an"` and explicit non-zero `step`                                               | reject  |
/// | `type != "an"` and `step` missing                                                         | reject  |
/// | `timespan` missing and `stattype` present                                                 | reject  |
/// | `timespan` missing and `allowMissingTimespanForInstantProduct == false`                   | reject  |
/// | `timespan = none` and `stattype` has more than one block                                  | reject  |
/// | `timespan = none` and fake-double-loop is not allowed for `(class, stream)`               | reject  |
/// | `timespan = duration`, `stattype = 0`, fake-double-loop is required for `(class, stream)` | reject  |
/// | `timespan = from-start` and `stattype` present                                            | reject  |
/// | `timespan = from-start`, resolved `step == 0`, and `allowZeroLengthFsWindow == false`     | reject  |
///
/// `FakeSingleLoopDoubleLoop` is a reserved shape for index-statistics products
/// whose source syntax looks like a standard single-loop statistic but whose
/// canonical representation contains two windows of the same size. The policy
/// hook exists now; the identifying helper currently returns `false` until the
/// exact `(type, class, paramId)` domain is finalized.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <cstddef>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecShapeClassification_details.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {

///
/// @brief Structural temporal-support shape resolved from normalized input.
///
/// The classification describes the canonical structural interpretation of the
/// product's temporal support and window layout.
///
enum class ProductTimeSpecShapeKind : std::size_t {
    Instant,
    StandardSingleLoop,
    MultiLoop,
    FakeDoubleLoopSingleLoop,
    FromStartSingleLoop,
    FakeSingleLoopDoubleLoop
};

///
/// @brief Classify the structural ProductTimeSpec shape from normalized input.
///
/// This stage decides whether the product is instant, standard single-loop,
/// multi-loop, fake-double-loop single-loop, from-start single-loop, or the
/// reserved fake-single-loop/double-loop index-statistics case.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return The resolved `ProductTimeSpecShapeKind` classification.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         classification failure, with `input.to_json()` attached as context.
///
template <class Input_t>
ProductTimeSpecShapeKind classify_ProductTimeSpecShape_or_throw(const Input_t& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        detail::checkProductTimeSpecShapeStepConsistency_or_throw(input);

        const std::size_t stattypeBlockCount = detail::countProductTimeSpecShapeStatTypeBlocks(input);
        const deductions::TimespanKind timespanKind = detail::productTimeSpecShapeTimespanKind(input);
        const bool requiresFakeDoubleLoop =
            detail::requiresFakeDoubleLoopRepresentation(input.marsClass, input.marsStream);

        switch (timespanKind) {
            case deductions::TimespanKind::Missing:
                if (stattypeBlockCount != 0) {
                    throw Mars2GribModelException(
                        "`stattype` is present while `timespan` is missing in normalized input",
                        input.to_json(),
                        Here());
                }
                if (!input.allowMissingTimespanForInstantProduct) {
                    throw Mars2GribModelException(
                        "Missing `timespan` instant compatibility is disabled in normalized ProductTimeSpec input",
                        input.to_json(),
                        Here());
                }
                return ProductTimeSpecShapeKind::Instant;

            case deductions::TimespanKind::None:
                if (stattypeBlockCount == 0) {
                    return ProductTimeSpecShapeKind::Instant;
                }
                if (stattypeBlockCount > 1) {
                    throw Mars2GribModelException(
                        "`timespan=none` supports at most one parsed `stattype` block",
                        input.to_json(),
                        Here());
                }
                if (!requiresFakeDoubleLoop) {
                    throw Mars2GribModelException(
                        "Fake-double-loop representation is not allowed for this `(class, stream)` pair",
                        input.to_json(),
                        Here());
                }
                return ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop;

            case deductions::TimespanKind::Duration:
                if (stattypeBlockCount == 0) {
                    if (requiresFakeDoubleLoop) {
                        throw Mars2GribModelException(
                            "Standard single-loop representation is forbidden because fake-double-loop is required for this `(class, stream)` pair",
                            input.to_json(),
                            Here());
                    }
                    if (detail::is_FakeSingleLoopDoubleLoop(input)) {
                        return ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop;
                    }
                    return ProductTimeSpecShapeKind::StandardSingleLoop;
                }
                return ProductTimeSpecShapeKind::MultiLoop;

            case deductions::TimespanKind::FromStart:
                if (stattypeBlockCount != 0) {
                    throw Mars2GribModelException(
                        "From-start products forbid parsed `stattype` blocks",
                        input.to_json(),
                        Here());
                }
                if (detail::isZeroProductTimeSpecResolvedStep(input) &&
                    !input.allowZeroLengthFsWindow) {
                    throw Mars2GribModelException(
                        "Zero-length from-start window is disabled in normalized ProductTimeSpec input",
                        input.to_json(),
                        Here());
                }
                return ProductTimeSpecShapeKind::FromStartSingleLoop;
        }

        throw Mars2GribModelException(
            "Unhandled normalized `timespan` classification while resolving ProductTimeSpec shape",
            input.to_json(),
            Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to classify `ProductTimeSpecShape` from normalized input",
            input.to_json(),
            Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models

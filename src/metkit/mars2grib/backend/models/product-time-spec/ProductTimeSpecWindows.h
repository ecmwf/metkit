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
/// @file ProductTimeSpecWindows.h
/// @brief Public windows-build surface for ProductTimeSpec.
///
/// Exposes the windows-build public model API:
/// - `ProductTimeSpecWindow`, one canonical statistical window;
/// - `ProductTimeSpecWindows`, the ordered canonical window sequence;
/// - `build_ProductTimeSpecWindows_or_throw(...)`.
///
/// This header intentionally stays small. It owns only the public window build
/// artifacts, the public build entry point, and the normative documentation of
/// canonical window materialization. Internal helper logic lives in
/// `detail/productTimeSpecWindows_details.h`.
///
/// Canonical windows are ordered from outermost to innermost. Each window owns:
/// - one statistical processing type;
/// - one time range;
/// - one time increment.
///
/// The construction rules are:
///
/// | `ProductTimeSpecShapeKind` | Window count          | Outermost processing                   | Innermost processing                   | Range source                              | Increment source                                                                                  |
/// |----------------------------|-----------------------|----------------------------------------|----------------------------------------|-------------------------------------------|---------------------------------------------------------------------------------------------------|
/// | `Instant`                  | `1` placeholder       | `Missing`                              | `Missing`                              | synthetic zero range                      | zero sentinel                                                                                     |
/// | `StandardSingleLoop`       | `1`                   | `innerMostTypeOfStatisticalProcessing` | same                                   | `timespan.duration`                       | classified increment                                                                              |
/// | `MultiLoop`                | `stattype.size() + 1` | first parsed `stattype` processing     | `innerMostTypeOfStatisticalProcessing` | parsed outer ranges + `timespan.duration` | outer windows use the immediately inner range; the innermost window uses the classified increment |
/// | `FakeDoubleLoopSingleLoop` | `1`                   | parsed `stattype[0]` processing        | same                                   | parsed `stattype[0]` range                | classified increment                                                                              |
/// | `FromStartSingleLoop`      | `1`                   | `innerMostTypeOfStatisticalProcessing` | same                                   | resolved `step`                           | classified increment                                                                              |
/// | `FakeSingleLoopDoubleLoop` | `2`                   | `IndexProcessing`                      | `innerMostTypeOfStatisticalProcessing` | `timespan.duration` on both windows       | same classified increment on both windows                                                         |
///
/// The windows builder performs local structural validation before the final
/// whole-object consistency stage:
///
/// | Condition                                         | Outcome |
/// |---------------------------------------------------|---------|
/// | non-instant window has `Missing` processing       | reject  |
/// | non-instant real window has non-positive range    | reject  |
/// | explicit/defaulted real increment is non-positive | reject  |
/// | shape-specific range source is absent             | reject  |
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <vector>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecWindows_details.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDataTypes.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib::backend::models {

///
/// @brief Build the canonical window sequence from input, classifications, and anchor.
///
/// The windows build stage receives the normalized input snapshot, all resolved
/// classifications, and the already-built anchor artifact. The current window
/// materialization rules depend primarily on the resolved shape and increment
/// classifications, but the complete set of classifications is accepted as part
/// of the uniform ProductTimeSpec build pipeline.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] anchorType Previously resolved anchor classification.
/// @param[in] shapeType Previously resolved shape classification.
/// @param[in] incrementType Previously resolved time-increment classification.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @return Fully resolved `ProductTimeSpecWindows` artifact.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         materialization or validation failure, with `input.to_json()`
///         attached as context.
///
template <class Input_t>
ProductTimeSpecWindows build_ProductTimeSpecWindows_or_throw(const Input_t& input,
                                                             TimeAnchorKind anchorType,
                                                             ProductTimeSpecShapeKind shapeType,
                                                             TimeIncrementKind incrementType,
                                                             const ProductTimeSpecAnchor& anchor) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        detail::checkProductTimeSpecWindowsAnchorConsistency_or_throw(
            input,
            anchorType,
            anchor);

        std::vector<ProductTimeSpecWindow> values;

        switch (shapeType) {
            case ProductTimeSpecShapeKind::Instant:
                values = detail::buildProductTimeSpecWindowsInstant_or_throw();
                break;

            case ProductTimeSpecShapeKind::StandardSingleLoop:
                values = detail::buildProductTimeSpecWindowsStandardSingleLoop_or_throw(input, incrementType);
                break;

            case ProductTimeSpecShapeKind::MultiLoop:
                values = detail::buildProductTimeSpecWindowsMultiLoop_or_throw(input, incrementType);
                break;

            case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
                values = detail::buildProductTimeSpecWindowsFakeDoubleLoopSingleLoop_or_throw(input, incrementType);
                break;

            case ProductTimeSpecShapeKind::FromStartSingleLoop:
                values = detail::buildProductTimeSpecWindowsFromStartSingleLoop_or_throw(input, incrementType);
                break;

            case ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop:
                values = detail::buildProductTimeSpecWindowsFakeSingleLoopDoubleLoop_or_throw(input, incrementType);
                break;
        }

        detail::checkProductTimeSpecWindowsLocalConsistency_or_throw(
            input,
            shapeType,
            incrementType,
            values);

        return ProductTimeSpecWindows{std::move(values)};
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to build `ProductTimeSpecWindows` from normalized input",
            input.to_json(),
            Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models

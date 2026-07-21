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
/// @file ProductTimeSpecTimeIncrementClassification.h
/// @brief Public time-increment classification surface for ProductTimeSpec.
///
/// Exposes the time-increment-classification public model API:
/// - `TimeIncrementKind`, the increment classification enum;
/// - `classify_ProductTimeSpecTimeIncrement_or_throw(...)`.
///
/// This header owns only the public increment-classification type, the public
/// classification entry point, and the normative documentation of the
/// increment-classification model. Internal helper logic lives in
/// `detail/productTimeSpecTimeIncrementClassification_details.h`.
///
/// Time-increment classification determines whether the product has no usable
/// increment, an explicit increment, a policy-defaulted increment, or the
/// AIFS-pure missing-increment sentinel case.
///
/// The primary classification table is:
///
/// | Shape kind               | Explicit increment | AIFS single-window case (`class=="ml"` and `realWindowCount==1`) | Defaulting allowed | Result                                                                         |
/// |--------------------------|--------------------|------------------------------------------------------------------|--------------------|--------------------------------------------------------------------------------|
/// | `Instant`                | no                 | no                                                               | n/a                | `NoIncrement`                                                                  |
/// | `Instant`                | yes                | no                                                               | n/a                | `NoIncrement` if redundant increment is allowed, otherwise reject              |
/// | non-`Instant`            | no                 | yes                                                              | n/a                | `AifsPureMissingIncrement`                                                     |
/// | non-`Instant`            | yes                | yes                                                              | n/a                | `AifsPureMissingIncrement` if redundant increment is allowed, otherwise reject |
/// | non-`Instant`            | yes                | no                                                               | n/a                | `ExplicitIncrement`                                                            |
/// | `FromStartSingleLoop`    | no                 | no                                                               | any                | reject                                                                         |
/// | non-`ml`, non-from-start | no                 | no                                                               | yes                | `DefaultedIncrement`                                                           |
/// | non-`ml`, non-from-start | no                 | no                                                               | no                 | reject                                                                         |
///
/// Redundant-increment acceptance and defaulted-increment eligibility are driven
/// by the boolean policy fields already stored in the normalized backend-model
/// input snapshot.
///
/// Final semantic relationships across shape, increment, and statistical
/// processing remain the responsibility of the later cross-classification
/// consistency stage.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <cstddef>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecTimeIncrementClassification_details.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {

///
/// @brief Semantic classification of the innermost sampling increment.
///
/// The classification distinguishes absence of any real increment, explicit
/// source increments, policy-defaulted increments, and the AIFS single-window
/// missing-increment sentinel case.
///
enum class TimeIncrementKind : std::size_t {
    NoIncrement,
    ExplicitIncrement,
    DefaultedIncrement,
    AifsPureMissingIncrement
};

///
/// @brief Classify ProductTimeSpec time-increment semantics from normalized input.
///
/// The classification depends on the already-resolved shape classification, the
/// presence of an explicit normalized increment, the AIFS `class="ml"`
/// single-window special case, and the normalized policy fields controlling
/// redundant and defaulted increments.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @return The resolved `TimeIncrementKind` classification.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         classification failure, with `input.to_json()` attached as context.
///
template <class Input_t>
TimeIncrementKind classify_ProductTimeSpecTimeIncrement_or_throw(const Input_t& input,
                                                                 ProductTimeSpecShapeKind shapeType) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasExplicitIncrement = detail::hasExplicitProductTimeSpecTimeIncrement(input);
        const std::size_t realWindowCount =
            detail::countRealProductTimeSpecStatisticalWindows(input, shapeType);

        if (shapeType == ProductTimeSpecShapeKind::Instant) {
            if (hasExplicitIncrement &&
                !input.allowRedundantTimeIncrement) {
                throw Mars2GribModelException(
                    "Instant product carries an explicit redundant normalized time increment",
                    input.to_json(),
                    Here());
            }
            return TimeIncrementKind::NoIncrement;
        }

        if (input.marsClass == "ml" && realWindowCount == 1) {
            if (hasExplicitIncrement &&
                !input.allowRedundantTimeIncrement) {
                throw Mars2GribModelException(
                    "AIFS-pure single-window product carries an explicit redundant normalized time increment",
                    input.to_json(),
                    Here());
            }
            return TimeIncrementKind::AifsPureMissingIncrement;
        }

        if (hasExplicitIncrement) {
            return TimeIncrementKind::ExplicitIncrement;
        }

        if (input.marsClass != "ml" &&
            shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop) {
            throw Mars2GribModelException(
                "Non-ml from-start product requires an explicit normalized time increment",
                input.to_json(),
                Here());
        }

        if (input.marsClass != "ml" &&
            input.allowDefaultTimeIncrementInSeconds) {
            return TimeIncrementKind::DefaultedIncrement;
        }

        throw Mars2GribModelException(
            "Statistical product has no usable normalized time increment",
            input.to_json(),
            Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to classify `ProductTimeSpec` time increment from normalized input",
            input.to_json(),
            Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models

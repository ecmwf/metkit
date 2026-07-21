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
/// @file productTimeSpecTimeIncrementClassification_details.h
/// @brief Internal helpers for ProductTimeSpec time-increment classification.
///

#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Test whether normalized model input carries an explicit time increment.
///
/// Time-increment classification distinguishes source-explicit increments from
/// policy-derived or semantically missing increments. The normalized model input
/// preserves source absence through `std::optional`, so explicit presence is a
/// simple optional-engagement check.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return `true` when a normalized explicit increment is present.
///
template <class Input_t>
bool hasExplicitProductTimeSpecTimeIncrement(const Input_t& input) {
    return input.timeIncrement.has_value();
}

///
/// @brief Count real statistical windows implied by a shape classification.
///
/// This helper is the model-layer counterpart of the real-window counting logic
/// used by increment classification before canonical windows exist.
///
/// The current structural counts are:
/// - `Instant` -> `0`;
/// - `StandardSingleLoop` -> `1`;
/// - `FakeDoubleLoopSingleLoop` -> `1`;
/// - `FromStartSingleLoop` -> `1`;
/// - `FakeSingleLoopDoubleLoop` -> `2`;
/// - `MultiLoop` -> parsed `stattype` block count plus one innermost window.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @return Number of real statistical windows implied by that shape.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         shape enum value is invalid for counting.
///
template <class Input_t>
std::size_t countRealProductTimeSpecStatisticalWindows(const Input_t& input,
                                                       ProductTimeSpecShapeKind shapeType) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    switch (shapeType) {
        case ProductTimeSpecShapeKind::Instant:
            return 0;
        case ProductTimeSpecShapeKind::StandardSingleLoop:
        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
        case ProductTimeSpecShapeKind::FromStartSingleLoop:
            return 1;
        case ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop:
            return 2;
        case ProductTimeSpecShapeKind::MultiLoop:
            return input.stattype.has_value() ? input.stattype->size() + 1 : 1;
    }

    throw Mars2GribModelException(
        "Invalid ProductTimeSpec shape classification while counting real statistical windows",
        input.to_json(),
        Here());
}

///
}  // namespace metkit::mars2grib::backend::models::detail

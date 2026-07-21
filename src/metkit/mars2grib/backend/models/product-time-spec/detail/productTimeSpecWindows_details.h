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
/// @file productTimeSpecWindows_details.h
/// @brief Internal helpers for ProductTimeSpec windows construction.
///

#pragma once

#include <vector>

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Return the canonical zero-duration sentinel used by the model.
///
/// The windows builder uses the same zero-second sentinel for instant-product
/// placeholder windows and for semantically missing increments.
///
/// @return Zero-second normalized duration.
///
inline deductions::TimeDuration buildProductTimeSpecWindowsZeroDuration_or_throw() {
    return deductions::TimeDuration{0, tables::TimeUnit::Second};
}

///
/// @brief Verify that the passed anchor classification matches the anchor artifact.
///
/// The windows builder receives both the classification enum and the already
/// built anchor artifact. This helper keeps the stage boundary consistent by
/// rejecting mismatched pairs before any window materialization happens.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] anchorType Previously resolved anchor classification.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         anchor artifact and classification disagree.
///
template <class Input_t>
void checkProductTimeSpecWindowsAnchorConsistency_or_throw(const Input_t& input,
                                                           TimeAnchorKind anchorType,
                                                           const ProductTimeSpecAnchor& anchor) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (anchor.anchorType != anchorType) {
        throw Mars2GribModelException(
            "ProductTimeSpec windows build received inconsistent anchor classification and anchor artifact",
            input.to_json(),
            Here());
    }
}

///
/// @brief Return the resolved `step` duration used by from-start window construction.
///
/// Shape classification already established whether the missing-step state is
/// legal. This helper therefore converts the optional step input into one
/// concrete normalized duration by returning the explicit step when present and
/// otherwise the zero-second analysis compatibility value.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return Explicit normalized `step`, or zero seconds when absent.
///
template <class Input_t>
deductions::TimeDuration buildProductTimeSpecWindowsResolvedStep_or_throw(const Input_t& input) {
    if (input.step.has_value()) {
        return *input.step;
    }

    return buildProductTimeSpecWindowsZeroDuration_or_throw();
}

///
/// @brief Build the innermost real window range for the active shape.
///
/// This helper centralizes the source selection for the innermost canonical
/// window range:
/// - `StandardSingleLoop` and `MultiLoop` use `timespan.duration`;
/// - `FromStartSingleLoop` uses the resolved `step` duration;
/// - `FakeDoubleLoopSingleLoop` uses the single parsed `stattype` range;
/// - `FakeSingleLoopDoubleLoop` uses `timespan.duration` for both equal windows;
/// - `Instant` uses the synthetic zero range.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @return The normalized innermost range for the active shape.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         required source range is absent.
///
template <class Input_t>
deductions::TimeDuration buildProductTimeSpecWindowsInnermostRange_or_throw(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    switch (shapeType) {
        case ProductTimeSpecShapeKind::Instant:
            return buildProductTimeSpecWindowsZeroDuration_or_throw();

        case ProductTimeSpecShapeKind::StandardSingleLoop:
        case ProductTimeSpecShapeKind::MultiLoop:
        case ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop:
            if (!input.timespan.has_value() || !input.timespan->duration.has_value()) {
                throw Mars2GribModelException(
                    "ProductTimeSpec windows construction requires normalized `timespan.duration` for this shape",
                    input.to_json(),
                    Here());
            }
            return *input.timespan->duration;

        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
            if (!input.stattype.has_value() || input.stattype->empty()) {
                throw Mars2GribModelException(
                    "ProductTimeSpec fake-double-loop windows construction requires one parsed `stattype` block",
                    input.to_json(),
                    Here());
            }
            return (*input.stattype)[0].timeRange;

        case ProductTimeSpecShapeKind::FromStartSingleLoop:
            return buildProductTimeSpecWindowsResolvedStep_or_throw(input);
    }

    throw Mars2GribModelException(
        "Unhandled ProductTimeSpec shape classification while selecting the innermost window range",
        input.to_json(),
        Here());
}

///
/// @brief Materialize the classified innermost time increment.
///
/// The windows builder needs one normalized increment value to place on the
/// innermost real window, and on both windows of the
/// `FakeSingleLoopDoubleLoop` case. The current rules are:
/// - `NoIncrement` -> zero-second sentinel;
/// - `AifsPureMissingIncrement` -> zero-second sentinel;
/// - `ExplicitIncrement` -> normalized explicit increment from input;
/// - `DefaultedIncrement` -> temporary model rule derived from the innermost
///   window range.
///
/// The current temporary defaulting rule is copied from the previous
/// implementation:
/// - inner range `< 1h` -> reject;
/// - inner range `== 1h` -> `600 seconds`;
/// - inner range `> 1h` -> `3600 seconds`;
/// - day/month ranges -> `3600 seconds`.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @param[in] incrementType Previously resolved valid increment classification.
/// @return The normalized innermost increment duration.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         explicit increment is missing where required or the temporary
///         defaulting rule cannot derive a valid increment.
///
template <class Input_t>
deductions::TimeDuration buildProductTimeSpecWindowsInnermostIncrement_or_throw(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType,
    TimeIncrementKind incrementType) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    switch (incrementType) {
        case TimeIncrementKind::NoIncrement:
        case TimeIncrementKind::AifsPureMissingIncrement:
            return buildProductTimeSpecWindowsZeroDuration_or_throw();

        case TimeIncrementKind::ExplicitIncrement:
            if (!input.timeIncrement.has_value()) {
                throw Mars2GribModelException(
                    "ProductTimeSpec explicit increment classification has no normalized explicit increment",
                    input.to_json(),
                    Here());
            }
            return *input.timeIncrement;

        case TimeIncrementKind::DefaultedIncrement: {
            const deductions::TimeDuration innerRange =
                buildProductTimeSpecWindowsInnermostRange_or_throw(input, shapeType);

            switch (innerRange.unit) {
                case tables::TimeUnit::Second:
                    if (innerRange.length < 3600) {
                        throw Mars2GribModelException(
                            "ProductTimeSpec defaulted increment requires an innermost range of at least one hour",
                            input.to_json(),
                            Here());
                    }
                    return innerRange.length == 3600
                               ? deductions::TimeDuration{600, tables::TimeUnit::Second}
                               : deductions::TimeDuration{3600, tables::TimeUnit::Second};

                case tables::TimeUnit::Hour:
                    if (innerRange.length < 1) {
                        throw Mars2GribModelException(
                            "ProductTimeSpec defaulted increment requires an innermost range of at least one hour",
                            input.to_json(),
                            Here());
                    }
                    return innerRange.length == 1
                               ? deductions::TimeDuration{600, tables::TimeUnit::Second}
                               : deductions::TimeDuration{1, tables::TimeUnit::Hour};

                case tables::TimeUnit::Day:
                case tables::TimeUnit::Month:
                    return deductions::TimeDuration{1, tables::TimeUnit::Hour};

                default:
                    throw Mars2GribModelException(
                        "ProductTimeSpec defaulted increment does not support this innermost range unit",
                        input.to_json(),
                        Here());
            }
        }
    }

    throw Mars2GribModelException(
        "Unhandled ProductTimeSpec increment classification while building windows",
        input.to_json(),
        Here());
}

///
/// @brief Validate local window invariants on the constructed canonical window sequence.
///
/// The windows builder performs local sequence checks before the final whole-
/// object consistency stage. The checks are:
/// - every non-instant window must have non-missing processing;
/// - every real non-zero-from-start range must be strictly positive;
/// - every non-sentinel real increment must be strictly positive.
///
/// The zero-length from-start case is allowed only when shape classification has
/// already accepted it and therefore appears here as a zero-valued resolved
/// `step` range.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @param[in] incrementType Previously resolved valid increment classification.
/// @param[in] windows Constructed canonical windows in outermost-to-innermost order.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if a
///         local window invariant is violated.
///
template <class Input_t, class Windows_t>
void checkProductTimeSpecWindowsLocalConsistency_or_throw(const Input_t& input,
                                                          ProductTimeSpecShapeKind shapeType,
                                                          TimeIncrementKind incrementType,
                                                          const Windows_t& windows) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    const bool zeroFromStart =
        shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop &&
        buildProductTimeSpecWindowsResolvedStep_or_throw(input).length == 0;
    const bool missingIncrementSentinel =
        incrementType == TimeIncrementKind::AifsPureMissingIncrement;

    for (const auto& window : windows) {
        if (shapeType != ProductTimeSpecShapeKind::Instant &&
            window.typeOfStatisticalProcessing == tables::TypeOfStatisticalProcessing::Missing) {
            throw Mars2GribModelException(
                "Real ProductTimeSpec window has `Missing` statistical processing",
                input.to_json(),
                Here());
        }

        if (shapeType != ProductTimeSpecShapeKind::Instant &&
            window.timeRange.length <= 0 && !zeroFromStart) {
            throw Mars2GribModelException(
                "Real ProductTimeSpec window has a non-positive range",
                input.to_json(),
                Here());
        }

        if (shapeType != ProductTimeSpecShapeKind::Instant &&
            !missingIncrementSentinel && incrementType != TimeIncrementKind::NoIncrement &&
            window.timeIncrement.length <= 0) {
            throw Mars2GribModelException(
                "Real ProductTimeSpec window has a non-positive materialized increment",
                input.to_json(),
                Here());
        }
    }
}

///
/// @brief Build the instant placeholder window sequence.
///
/// @return One placeholder window sequence for the instant-product case.
///
inline std::vector<ProductTimeSpecWindow> buildProductTimeSpecWindowsInstant_or_throw() {
    return std::vector<ProductTimeSpecWindow>{ProductTimeSpecWindow{
        tables::TypeOfStatisticalProcessing::Missing,
        buildProductTimeSpecWindowsZeroDuration_or_throw(),
        buildProductTimeSpecWindowsZeroDuration_or_throw()}};
}

///
/// @brief Build the canonical single-window sequence for the standard single-loop case.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] incrementType Previously resolved increment classification.
/// @return One canonical standard single-loop window.
///
template <class Input_t>
std::vector<ProductTimeSpecWindow> buildProductTimeSpecWindowsStandardSingleLoop_or_throw(
    const Input_t& input,
    TimeIncrementKind incrementType) {
    return std::vector<ProductTimeSpecWindow>{ProductTimeSpecWindow{
        input.innerMostTypeOfStatisticalProcessing,
        buildProductTimeSpecWindowsInnermostRange_or_throw(input, ProductTimeSpecShapeKind::StandardSingleLoop),
        buildProductTimeSpecWindowsInnermostIncrement_or_throw(input, ProductTimeSpecShapeKind::StandardSingleLoop, incrementType)}};
}

///
/// @brief Build the canonical single-window sequence for the fake-double-loop single-loop case.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] incrementType Previously resolved increment classification.
/// @return One canonical fake-double-loop single-loop window.
///
template <class Input_t>
std::vector<ProductTimeSpecWindow> buildProductTimeSpecWindowsFakeDoubleLoopSingleLoop_or_throw(
    const Input_t& input,
    TimeIncrementKind incrementType) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (!input.stattype.has_value() || input.stattype->empty()) {
        throw Mars2GribModelException(
            "ProductTimeSpec fake-double-loop windows construction requires one parsed `stattype` block",
            input.to_json(),
            Here());
    }

    return std::vector<ProductTimeSpecWindow>{ProductTimeSpecWindow{
        (*input.stattype)[0].typeOfStatisticalProcessing,
        (*input.stattype)[0].timeRange,
        buildProductTimeSpecWindowsInnermostIncrement_or_throw(input, ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop, incrementType)}};
}

///
/// @brief Build the canonical single-window sequence for the from-start case.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] incrementType Previously resolved increment classification.
/// @return One canonical from-start single-loop window.
///
template <class Input_t>
std::vector<ProductTimeSpecWindow> buildProductTimeSpecWindowsFromStartSingleLoop_or_throw(
    const Input_t& input,
    TimeIncrementKind incrementType) {
    return std::vector<ProductTimeSpecWindow>{ProductTimeSpecWindow{
        input.innerMostTypeOfStatisticalProcessing,
        buildProductTimeSpecWindowsInnermostRange_or_throw(input, ProductTimeSpecShapeKind::FromStartSingleLoop),
        buildProductTimeSpecWindowsInnermostIncrement_or_throw(input, ProductTimeSpecShapeKind::FromStartSingleLoop, incrementType)}};
}

///
/// @brief Build the canonical multi-loop window sequence.
///
/// Outer windows are copied from the parsed `stattype` blocks in their stored
/// outermost-to-innermost order. Each outer window uses the immediately inner
/// range as its increment. The final innermost window uses the caller-supplied
/// innermost statistical processing type, `timespan.duration`, and the
/// classified innermost increment.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] incrementType Previously resolved increment classification.
/// @return Canonical multi-loop window sequence in outermost-to-innermost order.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         parsed `stattype` sequence required by the shape is absent.
///
template <class Input_t>
std::vector<ProductTimeSpecWindow> buildProductTimeSpecWindowsMultiLoop_or_throw(
    const Input_t& input,
    TimeIncrementKind incrementType) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (!input.stattype.has_value() || input.stattype->empty()) {
        throw Mars2GribModelException(
            "ProductTimeSpec multi-loop windows construction requires parsed `stattype` blocks",
            input.to_json(),
            Here());
    }

    std::vector<ProductTimeSpecWindow> result;
    result.reserve(input.stattype->size() + 1);

    for (std::size_t i = 0; i < input.stattype->size(); ++i) {
        const deductions::TimeDuration innerRange =
            (i + 1 < input.stattype->size())
                ? (*input.stattype)[i + 1].timeRange
                : buildProductTimeSpecWindowsInnermostRange_or_throw(input, ProductTimeSpecShapeKind::MultiLoop);

        result.push_back(ProductTimeSpecWindow{
            (*input.stattype)[i].typeOfStatisticalProcessing,
            (*input.stattype)[i].timeRange,
            innerRange});
    }

    result.push_back(ProductTimeSpecWindow{
        input.innerMostTypeOfStatisticalProcessing,
        buildProductTimeSpecWindowsInnermostRange_or_throw(input, ProductTimeSpecShapeKind::MultiLoop),
        buildProductTimeSpecWindowsInnermostIncrement_or_throw(input, ProductTimeSpecShapeKind::MultiLoop, incrementType)});

    return std::vector<ProductTimeSpecWindow>{std::move(result)};
}

///
/// @brief Build the canonical two-window sequence for the fake-single-loop/double-loop case.
///
/// This reserved index-statistics shape materializes two equal windows:
/// - an outer synthetic window with `IndexProcessing`;
/// - an inner window with the caller-supplied innermost statistical processing.
///
/// Both windows use the same normalized `timespan.duration` as their range and
/// the same classified innermost increment as their increment.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] incrementType Previously resolved increment classification.
/// @return Canonical two-window sequence in outermost-to-innermost order.
///
template <class Input_t>
std::vector<ProductTimeSpecWindow> buildProductTimeSpecWindowsFakeSingleLoopDoubleLoop_or_throw(
    const Input_t& input,
    TimeIncrementKind incrementType) {
    const deductions::TimeDuration sharedRange =
        buildProductTimeSpecWindowsInnermostRange_or_throw(input, ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop);
    const deductions::TimeDuration sharedIncrement =
        buildProductTimeSpecWindowsInnermostIncrement_or_throw(input, ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop, incrementType);

    return std::vector<ProductTimeSpecWindow>{
        ProductTimeSpecWindow{
            tables::TypeOfStatisticalProcessing::IndexProcessing,
            sharedRange,
            sharedIncrement},
        ProductTimeSpecWindow{
            input.innerMostTypeOfStatisticalProcessing,
            sharedRange,
            sharedIncrement}};
}

}  // namespace metkit::mars2grib::backend::models::detail

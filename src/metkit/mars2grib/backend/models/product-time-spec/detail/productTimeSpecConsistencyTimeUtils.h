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
/// @file productTimeSpecConsistencyTimeUtils.h
/// @brief Calendar-aware time utilities for final ProductTimeSpec consistency validation.
///
/// This header contains header-only helper utilities used by the final
/// ProductTimeSpec consistency stage.
///
/// It owns:
/// - anchored `DateTime + TimeDuration` resolution helpers;
/// - boolean predicates for calendar-aware range ordering and exact
///   divisibility;
/// - model-level wrappers operating on `ProductTimeSpecDomain` and
///   `ProductTimeSpecWindow`;
/// - the normalized-input predicate for the explicit zero-length from-start
///   exception.
///
/// This header does NOT own:
/// - top-level final consistency checks;
/// - `Mars2GribModelException` construction;
/// - attachment of `input.to_json()` diagnostic context.
///
/// Standard exceptions raised by the lower-level calendar arithmetic are allowed
/// to propagate unchanged from this helper layer. The higher-level consistency
/// helpers remain responsible for wrapping those failures in the existing model
/// diagnostic structure.
///

#pragma once

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecWindows.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecTimeUtils.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Report whether a normalized ProductTimeSpec duration is zero-length.
///
/// The zero-length semantics used by final consistency are based only on the
/// stored integer length. The duration unit does not affect the result.
///
/// @param[in] duration Normalized ProductTimeSpec duration.
/// @return `true` when `duration.length == 0`, `false` otherwise.
///
inline bool isZeroProductTimeSpecDuration(const deductions::TimeDuration& duration) noexcept {
    return duration.length == 0;
}

///
/// @brief Report whether an increment classification represents a real semantic increment.
///
/// Final consistency treats explicit and defaulted increments as real
/// increments. The instant `NoIncrement` state and the AIFS missing-increment
/// sentinel do not carry semantic increment bounds.
///
/// @param[in] incrementType Final ProductTimeSpec increment classification.
/// @return `true` for `ExplicitIncrement` and `DefaultedIncrement`, `false`
///         otherwise.
///
inline bool hasSemanticProductTimeSpecIncrement(TimeIncrementKind incrementType) noexcept {
    return incrementType == TimeIncrementKind::ExplicitIncrement ||
           incrementType == TimeIncrementKind::DefaultedIncrement;
}

///
/// @brief Resolve one anchored ProductTimeSpec datetime by forward duration addition.
///
/// This helper is the consistency-layer entry point for calendar-aware
/// `anchorDateTime + duration` evaluation.
///
/// @param[in] anchorDateTime Absolute anchor datetime.
/// @param[in] duration Normalized ProductTimeSpec duration to add.
/// @return The resolved endpoint reached by adding `duration` to
///         `anchorDateTime`.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline eckit::DateTime resolveProductTimeSpecAnchoredDateTime_or_throw(
    const eckit::DateTime& anchorDateTime,
    const deductions::TimeDuration& duration) {
    return addProductTimeSpecDuration_or_throw(anchorDateTime, duration);
}

///
/// @brief Compare two durations by applying both to the same absolute anchor.
///
/// The predicate returns whether the endpoint produced by `outerDuration` is not
/// earlier than the endpoint produced by `innerDuration` when both are added to
/// the same `anchorDateTime`.
///
/// @param[in] anchorDateTime Absolute anchor datetime used for both durations.
/// @param[in] outerDuration Left-hand duration of the ordering check.
/// @param[in] innerDuration Right-hand duration of the ordering check.
/// @return `true` when
///         `anchorDateTime + outerDuration >= anchorDateTime + innerDuration`,
///         `false` otherwise.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline bool isProductTimeSpecAnchoredRangeOrdered_or_throw(
    const eckit::DateTime& anchorDateTime,
    const deductions::TimeDuration& outerDuration,
    const deductions::TimeDuration& innerDuration) {
    const eckit::DateTime outerEnd =
        resolveProductTimeSpecAnchoredDateTime_or_throw(anchorDateTime, outerDuration);
    const eckit::DateTime innerEnd =
        resolveProductTimeSpecAnchoredDateTime_or_throw(anchorDateTime, innerDuration);

    return outerEnd >= innerEnd;
}

///
/// @brief Test whether one duration is an exact sequential multiple of another.
///
/// The predicate applies both durations using calendar-aware arithmetic anchored
/// at `anchorDateTime`. Exact divisibility holds only when repeated sequential
/// addition of `innerDuration` lands exactly on the endpoint produced by one
/// application of `outerDuration`.
///
/// Overshooting the outer endpoint returns `false`. Semantic non-progress also
/// returns `false`; this occurs when one sequential addition of `innerDuration`
/// does not move the current datetime forward.
///
/// @param[in] anchorDateTime Absolute anchor datetime.
/// @param[in] outerDuration Duration defining the target outer endpoint.
/// @param[in] innerDuration Duration used as the repeated exact divisor.
/// @return `true` when repeated sequential addition of `innerDuration` reaches
///         exactly `anchorDateTime + outerDuration`, `false` otherwise.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline bool isProductTimeSpecAnchoredDurationExactlyDivisible_or_throw(
    const eckit::DateTime& anchorDateTime,
    const deductions::TimeDuration& outerDuration,
    const deductions::TimeDuration& innerDuration) {
    const eckit::DateTime targetEnd =
        resolveProductTimeSpecAnchoredDateTime_or_throw(anchorDateTime, outerDuration);

    eckit::DateTime current = anchorDateTime;
    if (current == targetEnd) {
        return true;
    }

    while (current < targetEnd) {
        const eckit::DateTime next =
            resolveProductTimeSpecAnchoredDateTime_or_throw(current, innerDuration);

        if (next <= current) {
            return false;
        }
        if (next == targetEnd) {
            return true;
        }
        if (next > targetEnd) {
            return false;
        }

        current = next;
    }

    return false;
}

///
/// @brief Resolve the absolute end datetime of one canonical window range.
///
/// The endpoint is evaluated by anchoring the window range at the resolved
/// `domainStartDateTime`.
///
/// @param[in] domain Resolved ProductTimeSpec domain artifact.
/// @param[in] window Canonical ProductTimeSpec window.
/// @return `domain.domainStartDateTime + window.timeRange`.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline eckit::DateTime resolveProductTimeSpecRangeEndDateTime_or_throw(
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindow& window) {
    return resolveProductTimeSpecAnchoredDateTime_or_throw(
        domain.domainStartDateTime,
        window.timeRange);
}

///
/// @brief Resolve the absolute end datetime of one canonical window increment.
///
/// The endpoint is evaluated by anchoring the window increment at the resolved
/// `domainStartDateTime`.
///
/// @param[in] domain Resolved ProductTimeSpec domain artifact.
/// @param[in] window Canonical ProductTimeSpec window.
/// @return `domain.domainStartDateTime + window.timeIncrement`.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline eckit::DateTime resolveProductTimeSpecIncrementEndDateTime_or_throw(
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindow& window) {
    return resolveProductTimeSpecAnchoredDateTime_or_throw(
        domain.domainStartDateTime,
        window.timeIncrement);
}

///
/// @brief Test whether the canonical outer window reproduces the resolved domain end.
///
/// This predicate performs the forward outermost-domain equality used by final
/// consistency:
/// `domainStartDateTime + outerWindow.timeRange == domainEndDateTime`.
///
/// @param[in] domain Resolved ProductTimeSpec domain artifact.
/// @param[in] outerWindow Outermost canonical ProductTimeSpec window.
/// @return `true` when the outer window reproduces `domain.domainEndDateTime`,
///         `false` otherwise.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline bool isProductTimeSpecOutermostDomainEqualitySatisfied_or_throw(
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindow& outerWindow) {
    return resolveProductTimeSpecRangeEndDateTime_or_throw(domain, outerWindow) ==
           domain.domainEndDateTime;
}

///
/// @brief Test whether one window increment is semantically bounded by its range.
///
/// The predicate performs the calendar-aware form of
/// `window.timeIncrement <= window.timeRange` by applying both durations to the
/// resolved `domainStartDateTime`.
///
/// @param[in] domain Resolved ProductTimeSpec domain artifact.
/// @param[in] window Canonical ProductTimeSpec window.
/// @return `true` when the range endpoint is not earlier than the increment
///         endpoint, `false` otherwise.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline bool isProductTimeSpecWindowIncrementBoundSatisfied_or_throw(
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindow& window) {
    return isProductTimeSpecAnchoredRangeOrdered_or_throw(
        domain.domainStartDateTime,
        window.timeRange,
        window.timeIncrement);
}

///
/// @brief Test whether an outer/inner window pair satisfies range hierarchy.
///
/// The predicate performs the calendar-aware form of
/// `outerWindow.timeRange >= innerWindow.timeRange` by applying both ranges to
/// the resolved `domainStartDateTime`.
///
/// @param[in] domain Resolved ProductTimeSpec domain artifact.
/// @param[in] outerWindow Outermost member of one adjacent pair.
/// @param[in] innerWindow Innermost member of one adjacent pair.
/// @return `true` when the outer range endpoint is not earlier than the inner
///         range endpoint, `false` otherwise.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline bool isProductTimeSpecWindowHierarchySatisfied_or_throw(
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindow& outerWindow,
    const ProductTimeSpecWindow& innerWindow) {
    return isProductTimeSpecAnchoredRangeOrdered_or_throw(
        domain.domainStartDateTime,
        outerWindow.timeRange,
        innerWindow.timeRange);
}

///
/// @brief Test whether an outer window range is exactly divisible by an inner range.
///
/// The predicate anchors both ranges at `domain.domainStartDateTime` and uses
/// sequential calendar-aware addition of `innerWindow.timeRange` to test whether
/// it lands exactly on the outer endpoint.
///
/// @param[in] domain Resolved ProductTimeSpec domain artifact.
/// @param[in] outerWindow Outermost member of one adjacent pair.
/// @param[in] innerWindow Innermost member of one adjacent pair.
/// @return `true` when `outerWindow.timeRange` is an exact sequential multiple
///         of `innerWindow.timeRange`, `false` otherwise.
/// @throws std::invalid_argument on invalid duration units, negative lengths, or
///         invalid calendar alignment.
/// @throws std::overflow_error if hour-to-second conversion overflows.
///
inline bool isProductTimeSpecWindowDivisibilitySatisfied_or_throw(
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindow& outerWindow,
    const ProductTimeSpecWindow& innerWindow) {
    return isProductTimeSpecAnchoredDurationExactlyDivisible_or_throw(
        domain.domainStartDateTime,
        outerWindow.timeRange,
        innerWindow.timeRange);
}

///
/// @brief Detect the explicit zero-length from-start final-consistency exception.
///
/// The predicate uses normalized input state rather than reverse-inferring the
/// exception from built artifacts. The exception is active only when:
/// - `shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop`;
/// - `input.allowZeroLengthFsWindow == true`;
/// - normalized `step` is present and has zero length.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Final ProductTimeSpec shape classification.
/// @return `true` when the explicit zero-length from-start exception is active,
///         `false` otherwise.
///
template <typename Input_t>
inline bool isProductTimeSpecZeroLengthFromStartException(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType) noexcept {
    return shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop &&
           input.allowZeroLengthFsWindow &&
           input.step.has_value() &&
           isZeroProductTimeSpecDuration(*input.step);
}

}  // namespace metkit::mars2grib::backend::models::detail

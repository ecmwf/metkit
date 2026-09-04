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
/// @file Instant.h
/// @brief Matcher, builders, and checker for the Instant shape.
///
/// This header is the authoritative implementation of the `Instant` shape
/// case. It keeps recognition, construction, and validation together so that
/// the complete case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the
/// canonical outer time range for this case. The final builder constructs the
/// one canonical instant window directly from visible local members. The
/// checker validates that the resolved shape remains consistent with both the
/// input semantics and the stage artifacts.
///
/// Every function catches all failures and rethrows a nested
/// `Mars2GribModelException` with the serialized input state.
///
/// @ingroup mars2grib_product_time_spec_shapes
///

#pragma once

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/TimeIncrement.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeUtils.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Return true only when input matches the Instant shape.
 *
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - `timespan` is explicitly `none`, or it is missing and the configured
 *   instant compatibility rule allows that source representation;
 * - no `stattype` blocks are present;
 * - the innermost statistical processing is missing.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_Instant_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) || timespanIsMissingAndAllowed(input, input.allowMissingTimespanForInstantProduct);
        const bool hasNoStattypeBlocks = input.stattype.empty();
        const bool hasMissingStatisticalProcessing =
            input.innerMostTypeOfStatisticalProcessing == TypeOfStatisticalProcessing::Missing;

        return isNotSeasonal && hasAcceptedTimespanRepresentation && hasNoStattypeBlocks &&
               hasMissingStatisticalProcessing;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_Instant_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the Instant shape.
 *
 * In this case:
 * - any redundant source increment must still be valid;
 * - the canonical outer time range is available immediately;
 * - the canonical outer time range is exactly zero duration.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_Instant_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::zeroDuration;

    try {
        (void)classification;

        // Instant products do not require a source increment. A present source
        // increment is therefore accepted only when redundant values are
        // explicitly enabled.
        const bool hasIncrement                = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;

        if (hasIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Instant timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(),
                Here());
        }

        const auto availability = ProductTimeSpecOuterTimeRangeAvailability::Available;
        const auto timeRange    = zeroDuration();

        return ProductTimeSpecOuterTimeRange{availability, timeRange};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_Instant_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the canonical Instant window.
 *
 * In this case:
 * - the innermost statistical processing is missing;
 * - the canonical time increment kind is missing;
 * - the canonical time range is zero duration;
 * - the canonical time increment is missing.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @return Constructed ProductTimeSpec shape for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecShape build_Instant_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::zeroDuration;

    try {
        (void)classification;
        (void)anchor;
        (void)outerTimeRange;
        (void)domain;

        // Instant products have no statistical-processing interval semantics.
        const TypeOfStatisticalProcessing typeOfStatisticalProcessing = TypeOfStatisticalProcessing::Missing;

        // Instant products therefore expose no increment-kind semantics.
        const auto typeOfTimeIncrement = missingTypeOfTimeIncrement();

        // The canonical instant window has zero temporal extent.
        const auto timeRange = zeroDuration();

        // Instant products do not encode a time increment value.
        const auto timeIncrement = missingIncrement();

        ProductTimeSpecWindow window{typeOfStatisticalProcessing, typeOfTimeIncrement, timeRange, timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_Instant_ShapeWindows`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved Instant shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `Instant`;
 * - the stage-1 outer time range is available and equals zero duration;
 * - the resolved shape contains exactly one canonical window;
 * - that window carries missing statistical-processing and increment semantics;
 * - the originating input still satisfies the Instant case semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the Instant case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_Instant_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const ProductTimeSpecShape& shape) {
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::zeroDuration;

    try {
        (void)anchor;
        (void)domain;

        if (classification.shapeType != ProductTimeSpecShapeKind::Instant) {
            throw Mars2GribModelException("Shape classification mismatch: expected Instant", input.to_json(), Here());
        }

        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) || timespanIsMissingAndAllowed(input, input.allowMissingTimespanForInstantProduct);
        const bool hasNoStattypeBlocks = input.stattype.empty();
        const bool hasMissingStatisticalProcessing =
            input.innerMostTypeOfStatisticalProcessing == TypeOfStatisticalProcessing::Missing;

        if (!isNotSeasonal || !hasAcceptedTimespanRepresentation || !hasNoStattypeBlocks ||
            !hasMissingStatisticalProcessing) {
            throw Mars2GribModelException("Instant input semantics are not satisfied", input.to_json(), Here());
        }

        if (outerTimeRange.availability != ProductTimeSpecOuterTimeRangeAvailability::Available) {
            throw Mars2GribModelException("Instant shape requires an available outer time range", input.to_json(),
                                          Here());
        }

        if (!outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("Instant shape requires a present outer time range value", input.to_json(),
                                          Here());
        }

        if (!compareTimeDuration(*outerTimeRange.timeRange, zeroDuration())) {
            throw Mars2GribModelException("Instant outer time range must be the canonical zero duration",
                                          input.to_json(), Here());
        }

        if (shape.values.size() != 1) {
            throw Mars2GribModelException("Instant shape must contain exactly one window", input.to_json(), Here());
        }

        const ProductTimeSpecWindow& window = shape.values.front();

        if (window.typeOfStatisticalProcessing != TypeOfStatisticalProcessing::Missing) {
            throw Mars2GribModelException("Instant window must use missing statistical processing", input.to_json(),
                                          Here());
        }

        if (window.typeOfTimeIncrement != TypeOfTimeIntervals::Missing) {
            throw Mars2GribModelException("Instant window must use missing typeOfTimeIncrement", input.to_json(),
                                          Here());
        }

        if (!compareTimeDuration(window.timeRange, zeroDuration())) {
            throw Mars2GribModelException("Instant window timeRange must be the canonical zero duration",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeIncrement, missingIncrement())) {
            throw Mars2GribModelException("Instant window timeIncrement must be missing", input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeRange, *outerTimeRange.timeRange)) {
            throw Mars2GribModelException("Instant window timeRange must match the outer time range", input.to_json(),
                                          Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_Instant_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

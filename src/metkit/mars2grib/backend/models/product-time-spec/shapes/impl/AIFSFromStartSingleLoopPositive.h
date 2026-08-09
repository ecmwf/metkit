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
/// @file AIFSFromStartSingleLoopPositive.h
/// @brief Matcher, builders, and checker for the AIFSFromStartSingleLoopPositive shape.
///
/// This header is the authoritative implementation of the
/// `AIFSFromStartSingleLoopPositive` shape case. It keeps recognition,
/// construction, and validation together so that the complete case can be
/// reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder keeps the outer time
/// range deferred for this case. The final builder constructs the one canonical
/// from-start AIFS statistical window from visible local members. The checker
/// validates that the resolved shape remains consistent with both the input
/// semantics and the stage artifacts.
///
/// Every function catches all failures and rethrows a nested
/// `Mars2GribModelException` with the serialized input state.
///
/// @ingroup mars2grib_product_time_spec_shapes
///
#pragma once

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/TimeIncrement.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Return true only when input matches the AIFSFromStartSingleLoopPositive shape.
 *
 * - the regime is AIFS;
 * - the product is forecast;
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - the product is not synoptic;
 * - the source increment is missing;
 * - `timespan` uses from-start semantics;
 * - no outer `stattype` blocks are present;
 * - step is strictly positive.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_AIFSFromStartSingleLoopPositive_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::detail::stepIsPositive;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isAifs     = input.regime == SimulationRegime::AIFS;
        const bool isForecast = input.simulationType == SimulationType::Forecast;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isNotSynoptic            = !input.isSynoptic;
        const bool sourceIncrementIsMissing = !input.timeIncrement.has_value();
        const bool usesFromStartTimespan    = input.timespan.kind == TimespanKind::FromStart;
        const bool hasNoStattypeBlocks      = input.stattype.empty();
        const bool hasPositiveStep          = stepIsPositive(input);

        return isAifs && isForecast && isNotSeasonal && isNotSynoptic && sourceIncrementIsMissing &&
               usesFromStartTimespan && hasNoStattypeBlocks && hasPositiveStep;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `match_AIFSFromStartSingleLoopPositive_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the AIFSFromStartSingleLoopPositive shape.
 *
 * In this case:
 * - the final range depends on the resolved absolute domain span;
 * - the stage-1 outer time range therefore remains deferred.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_AIFSFromStartSingleLoopPositive_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        const auto availability = ProductTimeSpecOuterTimeRangeAvailability::Deferred;

        return ProductTimeSpecOuterTimeRange{availability, std::nullopt};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_AIFSFromStartSingleLoopPositive_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the canonical AIFS from-start positive-step window.
 *
 * In this case:
 * - the canonical time range is the resolved domain span;
 * - the statistical processing type is the innermost normalized processing;
 * - the canonical time increment kind is missing;
 * - the canonical time increment value is missing;
 * - exactly one canonical window is produced.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @return Constructed ProductTimeSpec shape for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecShape build_AIFSFromStartSingleLoopPositive_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::durationBetween;

    try {
        (void)classification;
        (void)anchor;

        const bool outerTimeRangeIsDeferred =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Deferred;

        if (!outerTimeRangeIsDeferred || outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive requires a deferred outer time range",
                                          input.to_json(), Here());
        }

        if (input.timeIncrement.has_value()) {
            throw Mars2GribModelException("AIFS statistics require timeIncrementInSeconds to be missing",
                                          input.to_json(), Here());
        }

        const TimeDuration timeRange = durationBetween(domain.domainStartDateTime, domain.domainEndDateTime);

        // This case carries the normalized innermost statistical processing
        // directly into the one canonical window.
        const auto typeOfStatisticalProcessing = input.innerMostTypeOfStatisticalProcessing;

        // Pure AIFS from-start products always encode missing increment-kind semantics.
        const auto typeOfTimeIncrement = missingTypeOfTimeIncrement();

        // The canonical range is the real domain span for the positive from-start case.
        const auto canonicalTimeRange = timeRange;

        // Pure AIFS from-start products always encode a missing increment value.
        const auto timeIncrement = missingIncrement();

        ProductTimeSpecWindow window{typeOfStatisticalProcessing, typeOfTimeIncrement, canonicalTimeRange,
                                     timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_AIFSFromStartSingleLoopPositive_ShapeWindows`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved AIFSFromStartSingleLoopPositive shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `AIFSFromStartSingleLoopPositive`;
 * - the originating input still satisfies the case semantics;
 * - the stage-1 outer time range remains deferred;
 * - the resolved shape contains exactly one canonical window;
 * - that window matches the locally recomputed positive-step from-start AIFS semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the AIFSFromStartSingleLoopPositive case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_AIFSFromStartSingleLoopPositive_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const ProductTimeSpecShape& shape) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::stepIsPositive;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecWindow;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::durationBetween;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive) {
            throw Mars2GribModelException("Shape classification mismatch: expected AIFSFromStartSingleLoopPositive",
                                          input.to_json(), Here());
        }

        const bool isAifs     = input.regime == SimulationRegime::AIFS;
        const bool isForecast = input.simulationType == SimulationType::Forecast;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isNotSynoptic            = !input.isSynoptic;
        const bool sourceIncrementIsMissing = !input.timeIncrement.has_value();
        const bool usesFromStartTimespan    = input.timespan.kind == TimespanKind::FromStart;
        const bool hasNoStattypeBlocks      = input.stattype.empty();
        const bool hasPositiveStep          = stepIsPositive(input);

        if (!isAifs || !isForecast || !isNotSeasonal || !isNotSynoptic || !sourceIncrementIsMissing ||
            !usesFromStartTimespan || !hasNoStattypeBlocks || !hasPositiveStep) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive input semantics are not satisfied",
                                          input.to_json(), Here());
        }

        if (domain.isSynoptic) {
            throw Mars2GribModelException(
                "AIFSFromStartSingleLoopPositive shape must not be paired with a synoptic domain", input.to_json(),
                Here());
        }

        const bool outerTimeRangeIsDeferred =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Deferred;

        if (!outerTimeRangeIsDeferred || outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive requires a deferred outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration expectedTimeRange = durationBetween(domain.domainStartDateTime, domain.domainEndDateTime);

        if (shape.values.size() != 1) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive shape must contain exactly one window",
                                          input.to_json(), Here());
        }

        const ProductTimeSpecWindow& window = shape.values.front();

        if (window.typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "AIFSFromStartSingleLoopPositive window statistical processing does not match the innermost input "
                "processing",
                input.to_json(), Here());
        }

        if (window.typeOfTimeIncrement != missingTypeOfTimeIncrement()) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive window typeOfTimeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive window timeRange is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeIncrement, missingIncrement())) {
            throw Mars2GribModelException("AIFSFromStartSingleLoopPositive window timeIncrement must be missing",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `check_AIFSFromStartSingleLoopPositive_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

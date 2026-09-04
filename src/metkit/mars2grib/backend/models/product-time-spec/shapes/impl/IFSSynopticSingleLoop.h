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
/// @file IFSSynopticSingleLoop.h
/// @brief Matcher, builders, and checker for the IFSSynopticSingleLoop shape.
///
/// This header is the authoritative implementation of the
/// `IFSSynopticSingleLoop` shape case. It keeps recognition, construction, and
/// validation together so that the complete case can be reviewed without
/// following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the
/// intrinsic one-month outer time range directly in the callback. The final
/// builder constructs the one canonical synoptic-analysis window from visible
/// local members and locally resolved increment semantics. The checker
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
 * @brief Return true only when input matches the IFSSynopticSingleLoop shape.
 *
 * - the regime is IFS;
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - the product is synoptic;
 * - the product is analysis;
 * - no outer `stattype` blocks are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_IFSSynopticSingleLoop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isIfs = input.regime == SimulationRegime::IFS;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isSynoptic               = input.isSynoptic;
        const bool isAnalysis               = input.simulationType == SimulationType::Analysis;
        const bool hasNoStattypeBlocks      = input.stattype.empty();

        return isIfs && isNotSeasonal && isSynoptic && isAnalysis && hasNoStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_IFSSynopticSingleLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the IFSSynopticSingleLoop shape.
 *
 * In this case:
 * - the synoptic window range is intrinsic;
 * - the canonical outer time range is exactly one calendar month;
 * - the intrinsic range is available immediately.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_IFSSynopticSingleLoop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)classification;

        const auto availability      = ProductTimeSpecOuterTimeRangeAvailability::Available;
        const TimeDuration timeRange = oneMonth();

        return ProductTimeSpecOuterTimeRange{availability, timeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_IFSSynopticSingleLoop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the canonical IFS synoptic-analysis window.
 *
 * In this case:
 * - the statistical processing type is the innermost normalized processing;
 * - the canonical time increment kind is the synoptic analysis value;
 * - the canonical time range is one calendar month;
 * - the canonical time increment is twenty-four hours;
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
inline ProductTimeSpecShape build_IFSSynopticSingleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::analysisTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;
    using metkit::mars2grib::utils::time_arithmetic::twentyFourHours;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("IFSSynopticSingleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration expectedTimeRange = oneMonth();

        if (!compareTimeDuration(*outerTimeRange.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("IFSSynopticSingleLoop outer time range must be one calendar month",
                                          input.to_json(), Here());
        }

        constexpr long expectedIncrementInSeconds = 86400L;

        const bool hasExplicitIncrement        = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;
        const bool explicitIncrementHasExpectedValue =
            !hasExplicitIncrement || convertToSeconds(*input.timeIncrement) == expectedIncrementInSeconds;

        if (hasExplicitIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Synoptic timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(),
                Here());
        }

        if (!explicitIncrementHasExpectedValue) {
            throw Mars2GribModelException("Synoptic timeIncrementInSeconds must equal 86400", input.to_json(), Here());
        }

        // This case carries the normalized innermost statistical processing
        // directly into the one canonical window.
        const auto typeOfStatisticalProcessing = input.innerMostTypeOfStatisticalProcessing;

        // Synoptic analysis uses the dedicated analysis increment-kind code.
        const auto typeOfTimeIncrement = analysisTypeOfTimeIncrement();

        // The canonical range of a synoptic analysis window is one calendar month.
        const auto timeRange = expectedTimeRange;

        // The canonical synoptic increment is always twenty-four hours.
        const auto timeIncrement = twentyFourHours();

        ProductTimeSpecWindow window{typeOfStatisticalProcessing, typeOfTimeIncrement, timeRange, timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_IFSSynopticSingleLoop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved IFSSynopticSingleLoop shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `IFSSynopticSingleLoop`;
 * - the originating input still satisfies the case semantics;
 * - the stage-1 outer time range is available and equals one calendar month;
 * - the resolved shape contains exactly one canonical window;
 * - that window matches the locally recomputed synoptic-analysis semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the IFSSynopticSingleLoop case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_IFSSynopticSingleLoop_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const ProductTimeSpecShape& shape) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::analysisTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecWindow;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;
    using metkit::mars2grib::utils::time_arithmetic::twentyFourHours;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::IFSSynopticSingleLoop) {
            throw Mars2GribModelException("Shape classification mismatch: expected IFSSynopticSingleLoop",
                                          input.to_json(), Here());
        }

        const bool isIfs = input.regime == SimulationRegime::IFS;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isSynoptic               = input.isSynoptic;
        const bool isAnalysis               = input.simulationType == SimulationType::Analysis;
        const bool hasNoStattypeBlocks      = input.stattype.empty();

        if (!isIfs || !isNotSeasonal || !isSynoptic || !isAnalysis || !hasNoStattypeBlocks) {
            throw Mars2GribModelException("IFSSynopticSingleLoop input semantics are not satisfied", input.to_json(),
                                          Here());
        }

        if (!domain.isSynoptic) {
            throw Mars2GribModelException("IFSSynopticSingleLoop shape must be paired with a synoptic domain",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("IFSSynopticSingleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration expectedTimeRange = oneMonth();

        if (!compareTimeDuration(*outerTimeRange.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("IFSSynopticSingleLoop outer time range must be one calendar month",
                                          input.to_json(), Here());
        }

        if (shape.values.size() != 1) {
            throw Mars2GribModelException("IFSSynopticSingleLoop shape must contain exactly one window",
                                          input.to_json(), Here());
        }

        constexpr long expectedIncrementInSeconds = 86400L;

        const bool hasExplicitIncrement        = input.timeIncrement.has_value();
        const bool redundantIncrementIsAllowed = input.allowRedundantTimeIncrement;
        const bool explicitIncrementHasExpectedValue =
            !hasExplicitIncrement || convertToSeconds(*input.timeIncrement) == expectedIncrementInSeconds;

        if (hasExplicitIncrement && !redundantIncrementIsAllowed) {
            throw Mars2GribModelException(
                "Synoptic timeIncrementInSeconds is redundant but redundant values are disabled", input.to_json(),
                Here());
        }

        if (!explicitIncrementHasExpectedValue) {
            throw Mars2GribModelException("Synoptic timeIncrementInSeconds must equal 86400", input.to_json(), Here());
        }

        const ProductTimeSpecWindow& window = shape.values.front();

        if (window.typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "IFSSynopticSingleLoop window statistical processing does not match the innermost input processing",
                input.to_json(), Here());
        }

        if (window.typeOfTimeIncrement != analysisTypeOfTimeIncrement()) {
            throw Mars2GribModelException("IFSSynopticSingleLoop window typeOfTimeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("IFSSynopticSingleLoop window timeRange is inconsistent", input.to_json(),
                                          Here());
        }

        if (!compareTimeDuration(window.timeIncrement, twentyFourHours())) {
            throw Mars2GribModelException("IFSSynopticSingleLoop window timeIncrement must be twenty-four hours",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_IFSSynopticSingleLoop_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

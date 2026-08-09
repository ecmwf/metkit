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
/// @file IFSFakeSingleLoopDoubleLoop.h
/// @brief Matcher, builders, and checker for the IFSFakeSingleLoopDoubleLoop shape.
///
/// This header is the authoritative implementation of the
/// `IFSFakeSingleLoopDoubleLoop` shape case. It keeps recognition,
/// construction, and validation together so that the complete case can be
/// reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the
/// canonical outer time range directly from the normalized duration-valued
/// `timespan`. The final builder constructs the two canonical windows from
/// visible local members and locally resolved increment semantics. The checker
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
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Return true only when input matches the IFSFakeSingleLoopDoubleLoop shape.
 *
 * - the regime is IFS;
 * - the product is forecast;
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - the product is not synoptic;
 * - `timespan` is duration-valued;
 * - no outer `stattype` blocks are present;
 * - the product does not require the fake-double-loop compatibility case;
 * - the product requires the fake-single-loop-double-loop compatibility case.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_IFSFakeSingleLoopDoubleLoop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isIfs      = input.regime == SimulationRegime::IFS;
        const bool isForecast = input.simulationType == SimulationType::Forecast;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics     = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal                = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isNotSynoptic                = !input.isSynoptic;
        const bool hasDurationTimespan          = input.timespan.kind == TimespanKind::Duration;
        const bool hasNoStattypeBlocks          = input.stattype.empty();
        const bool requiresFakeDoubleLoop       = input.requiresFakeDoubleLoopSingleLoopRepresentation;
        const bool doesNotRequireFakeDoubleLoop = !requiresFakeDoubleLoop;
        const bool requiresFakeSecondLoop       = input.requiresFakeSingleLoopDoubleLoopRepresentation;

        return isIfs && isForecast && isNotSeasonal && isNotSynoptic && hasDurationTimespan && hasNoStattypeBlocks &&
               doesNotRequireFakeDoubleLoop && requiresFakeSecondLoop;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `match_IFSFakeSingleLoopDoubleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the IFSFakeSingleLoopDoubleLoop shape.
 *
 * In this case:
 * - `timespan` must be explicitly duration-valued;
 * - the duration value itself is the canonical outer time range;
 * - the original normalized duration unit is preserved.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_IFSFakeSingleLoopDoubleLoop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        if (input.timespan.kind != TimespanKind::Duration) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop requires a duration-valued timespan",
                                          input.to_json(), Here());
        }

        if (!input.timespan.duration.has_value()) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop duration-valued timespan must contain a duration", input.to_json(),
                Here());
        }

        const TimeDuration timeRange = *input.timespan.duration;
        const auto availability      = ProductTimeSpecOuterTimeRangeAvailability::Available;

        return ProductTimeSpecOuterTimeRange{availability, timeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_IFSFakeSingleLoopDoubleLoop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the two canonical windows of the IFS fake-single-loop double-loop shape.
 *
 * In this case:
 * - the normalized duration-valued `timespan` is shared by both canonical windows;
 * - the synthetic outer window uses `IndexProcessing`;
 * - the synthetic outer window increment equals the inner window range;
 * - the innermost window follows normal IFS single-loop increment semantics;
 * - exactly two canonical windows are produced in outermost-to-innermost order.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @return Constructed ProductTimeSpec shape for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecShape build_IFSFakeSingleLoopDoubleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::detail::deduceDefaultTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;

        if (input.timespan.kind != TimespanKind::Duration) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop requires a duration-valued timespan",
                                          input.to_json(), Here());
        }

        if (!input.timespan.duration.has_value()) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop duration-valued timespan must contain a duration", input.to_json(),
                Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration sharedTimeRange = *input.timespan.duration;

        if (!compareTimeDuration(sharedTimeRange, *outerTimeRange.timeRange)) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop timespan duration does not match the resolved outer time range",
                input.to_json(), Here());
        }

        ResolvedInnerIncrement resolvedInnermostIncrement{};

        if (input.timeIncrement.has_value()) {
            const long incrementInSeconds = convertToSeconds(*input.timeIncrement);

            if (incrementInSeconds <= 0) {
                throw Mars2GribModelException("Explicit timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (sharedTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(sharedTimeRange);

                if (incrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            resolvedInnermostIncrement.timeIncrement = TimeDuration{incrementInSeconds, TimeUnit::Second};
            resolvedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, sharedTimeRange);
        }
        else if (!input.allowDefaultTimeIncrement) {
            resolvedInnermostIncrement.timeIncrement       = missingIncrement();
            resolvedInnermostIncrement.typeOfTimeIncrement = missingTypeOfTimeIncrement();
        }
        else {
            throw Mars2GribModelException(
                "Default time-increment deduction for IFSFakeSingleLoopDoubleLoop is not implemented", input.to_json(),
                Here());

            const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, sharedTimeRange);

            if (defaultIncrementInSeconds <= 0) {
                throw Mars2GribModelException("Defaulted timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (sharedTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(sharedTimeRange);

                if (defaultIncrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            resolvedInnermostIncrement.timeIncrement = TimeDuration{defaultIncrementInSeconds, TimeUnit::Second};
            resolvedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, sharedTimeRange);
        }

        // The synthetic outer loop is the fake compatibility shell and therefore
        // uses IndexProcessing.
        const TypeOfStatisticalProcessing outerTypeOfStatisticalProcessing =
            TypeOfStatisticalProcessing::IndexProcessing;

        // The outer synthetic window uses the multi-loop outer increment-kind semantics.
        const auto outerTypeOfTimeIncrement = typeOfTimeIncrementForWindow(input, true, false, sharedTimeRange);

        // The outer synthetic window shares the same source range as the inner one.
        const auto outerTimeRangeValue = sharedTimeRange;

        // In this fake-single-loop case, the outer increment equals the inner range,
        // and the inner range equals the shared outer range.
        const auto outerTimeIncrement = sharedTimeRange;

        ProductTimeSpecWindow outerWindow{outerTypeOfStatisticalProcessing, outerTypeOfTimeIncrement,
                                          outerTimeRangeValue, outerTimeIncrement};

        // The innermost window preserves the real normalized statistical processing.
        const auto innerTypeOfStatisticalProcessing = input.innerMostTypeOfStatisticalProcessing;

        // The innermost increment kind follows the locally resolved IFS rules.
        const auto innerTypeOfTimeIncrement = resolvedInnermostIncrement.typeOfTimeIncrement;

        // The innermost range is the same shared source range.
        const auto innerTimeRange = sharedTimeRange;

        // The innermost increment is the explicit, missing, or defaulted resolved value.
        const auto innerTimeIncrement = resolvedInnermostIncrement.timeIncrement;

        ProductTimeSpecWindow innerWindow{innerTypeOfStatisticalProcessing, innerTypeOfTimeIncrement, innerTimeRange,
                                          innerTimeIncrement};

        return ProductTimeSpecShape{{outerWindow, innerWindow}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_IFSFakeSingleLoopDoubleLoop_ShapeWindows`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved IFSFakeSingleLoopDoubleLoop shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `IFSFakeSingleLoopDoubleLoop`;
 * - the originating input still satisfies the case semantics;
 * - the stage-1 outer time range matches the direct duration-valued `timespan`;
 * - the resolved shape contains exactly two canonical windows;
 * - the synthetic outer window uses `IndexProcessing` and its increment equals the inner range;
 * - the innermost window matches the locally recomputed IFS single-loop semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the IFSFakeSingleLoopDoubleLoop case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_IFSFakeSingleLoopDoubleLoop_Shape(
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
    using metkit::mars2grib::backend::models::product_time_spec::detail::deduceDefaultTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecWindow;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop) {
            throw Mars2GribModelException("Shape classification mismatch: expected IFSFakeSingleLoopDoubleLoop",
                                          input.to_json(), Here());
        }

        const bool isIfs      = input.regime == SimulationRegime::IFS;
        const bool isForecast = input.simulationType == SimulationType::Forecast;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics     = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal                = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isNotSynoptic                = !input.isSynoptic;
        const bool hasDurationTimespan          = input.timespan.kind == TimespanKind::Duration;
        const bool hasDurationValue             = input.timespan.duration.has_value();
        const bool hasNoStattypeBlocks          = input.stattype.empty();
        const bool requiresFakeDoubleLoop       = input.requiresFakeDoubleLoopSingleLoopRepresentation;
        const bool doesNotRequireFakeDoubleLoop = !requiresFakeDoubleLoop;
        const bool requiresFakeSecondLoop       = input.requiresFakeSingleLoopDoubleLoopRepresentation;

        if (!isIfs || !isForecast || !isNotSeasonal || !isNotSynoptic || !hasDurationTimespan || !hasDurationValue ||
            !hasNoStattypeBlocks || !doesNotRequireFakeDoubleLoop || !requiresFakeSecondLoop) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop input semantics are not satisfied",
                                          input.to_json(), Here());
        }

        if (domain.isSynoptic) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop shape must not be paired with a synoptic domain",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration sharedTimeRange = *input.timespan.duration;

        if (!compareTimeDuration(*outerTimeRange.timeRange, sharedTimeRange)) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop outer time range does not match the direct timespan duration",
                input.to_json(), Here());
        }

        if (shape.values.size() != 2) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop shape must contain exactly two windows",
                                          input.to_json(), Here());
        }

        ResolvedInnerIncrement expectedInnermostIncrement{};

        if (input.timeIncrement.has_value()) {
            const long incrementInSeconds = convertToSeconds(*input.timeIncrement);

            if (incrementInSeconds <= 0) {
                throw Mars2GribModelException("Explicit timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (sharedTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(sharedTimeRange);

                if (incrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            expectedInnermostIncrement.timeIncrement = TimeDuration{incrementInSeconds, TimeUnit::Second};
            expectedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, sharedTimeRange);
        }
        else if (!input.allowDefaultTimeIncrement) {
            expectedInnermostIncrement.timeIncrement       = missingIncrement();
            expectedInnermostIncrement.typeOfTimeIncrement = missingTypeOfTimeIncrement();
        }
        else {
            throw Mars2GribModelException(
                "Default time-increment deduction for IFSFakeSingleLoopDoubleLoop is not implemented", input.to_json(),
                Here());

            const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, sharedTimeRange);

            if (defaultIncrementInSeconds <= 0) {
                throw Mars2GribModelException("Defaulted timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (sharedTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(sharedTimeRange);

                if (defaultIncrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            expectedInnermostIncrement.timeIncrement = TimeDuration{defaultIncrementInSeconds, TimeUnit::Second};
            expectedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, sharedTimeRange);
        }

        const ProductTimeSpecWindow& outerWindow = shape.values[0];
        const ProductTimeSpecWindow& innerWindow = shape.values[1];

        if (outerWindow.typeOfStatisticalProcessing != TypeOfStatisticalProcessing::IndexProcessing) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop outer window must use IndexProcessing",
                                          input.to_json(), Here());
        }

        if (innerWindow.typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop inner window statistical processing does not match the innermost input "
                "processing",
                input.to_json(), Here());
        }

        if (outerWindow.typeOfTimeIncrement != typeOfTimeIncrementForWindow(input, true, false, sharedTimeRange)) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop outer window typeOfTimeIncrement is inconsistent", input.to_json(),
                Here());
        }

        if (innerWindow.typeOfTimeIncrement != expectedInnermostIncrement.typeOfTimeIncrement) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop inner window typeOfTimeIncrement is inconsistent", input.to_json(),
                Here());
        }

        if (!compareTimeDuration(outerWindow.timeRange, sharedTimeRange) ||
            !compareTimeDuration(innerWindow.timeRange, sharedTimeRange)) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop window ranges are inconsistent", input.to_json(),
                                          Here());
        }

        if (!compareTimeDuration(outerWindow.timeIncrement, innerWindow.timeRange)) {
            throw Mars2GribModelException(
                "IFSFakeSingleLoopDoubleLoop outer window increment must equal the inner window range", input.to_json(),
                Here());
        }

        if (!compareTimeDuration(innerWindow.timeIncrement, expectedInnermostIncrement.timeIncrement)) {
            throw Mars2GribModelException("IFSFakeSingleLoopDoubleLoop inner window timeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `check_IFSFakeSingleLoopDoubleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

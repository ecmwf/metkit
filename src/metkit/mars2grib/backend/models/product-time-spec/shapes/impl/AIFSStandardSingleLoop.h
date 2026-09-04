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
/// @file AIFSStandardSingleLoop.h
/// @brief Matcher, builders, and checker for the AIFSStandardSingleLoop shape.
///
/// This header is the authoritative implementation of the
/// `AIFSStandardSingleLoop` shape case. It keeps recognition, construction, and
/// validation together so that the complete case can be reviewed without
/// following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the
/// canonical outer time range directly from the normalized duration-valued
/// `timespan`. The final builder constructs the one canonical AIFS statistical
/// window from visible local members. The checker validates that the resolved
/// shape remains consistent with both the input semantics and the stage
/// artifacts.
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
 * @brief Return true only when input matches the AIFSStandardSingleLoop shape.
 *
 * - the regime is AIFS;
 * - the product is forecast;
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - the product is not synoptic;
 * - the source increment is missing;
 * - `timespan` is duration-valued;
 * - no outer `stattype` blocks are present;
 * - the product does not require the fake-double-loop compatibility case;
 * - the product does not require the fake-single-loop-double-loop compatibility case.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_AIFSStandardSingleLoop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isAifs     = input.regime == SimulationRegime::AIFS;
        const bool isForecast = input.simulationType == SimulationType::Forecast;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics               = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal                          = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isNotSynoptic                          = !input.isSynoptic;
        const bool sourceIncrementIsMissing               = !input.timeIncrement.has_value();
        const bool hasDurationTimespan                    = input.timespan.kind == TimespanKind::Duration;
        const bool hasNoStattypeBlocks                    = input.stattype.empty();
        const bool requiresFakeDoubleLoop                 = input.requiresFakeDoubleLoopSingleLoopRepresentation;
        const bool requiresFakeSecondLoop                 = input.requiresFakeSingleLoopDoubleLoopRepresentation;
        const bool doesNotRequireFakeDoubleLoop           = !requiresFakeDoubleLoop;
        const bool doesNotRequireFakeSingleLoopDoubleLoop = !requiresFakeSecondLoop;

        return isAifs && isForecast && isNotSeasonal && isNotSynoptic && sourceIncrementIsMissing &&
               hasDurationTimespan && hasNoStattypeBlocks && doesNotRequireFakeDoubleLoop &&
               doesNotRequireFakeSingleLoopDoubleLoop;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_AIFSStandardSingleLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the AIFSStandardSingleLoop shape.
 *
 * In this case:
 * - `timespan` must be explicitly duration-valued;
 * - the source increment must be absent;
 * - the duration value itself is the canonical outer time range;
 * - the original normalized duration unit is preserved.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_AIFSStandardSingleLoop_ShapeOuterTimeRange(
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
            throw Mars2GribModelException("AIFSStandardSingleLoop requires a duration-valued timespan", input.to_json(),
                                          Here());
        }

        if (!input.timespan.duration.has_value()) {
            throw Mars2GribModelException("AIFSStandardSingleLoop duration-valued timespan must contain a duration",
                                          input.to_json(), Here());
        }

        if (input.timeIncrement.has_value()) {
            throw Mars2GribModelException("AIFS statistics require timeIncrementInSeconds to be missing",
                                          input.to_json(), Here());
        }

        const TimeDuration timeRange = *input.timespan.duration;
        const auto availability      = ProductTimeSpecOuterTimeRangeAvailability::Available;

        return ProductTimeSpecOuterTimeRange{availability, timeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_AIFSStandardSingleLoop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the canonical AIFS single-loop window.
 *
 * In this case:
 * - the canonical time range is the normalized duration-valued `timespan`;
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
inline ProductTimeSpecShape build_AIFSStandardSingleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;

        if (input.timespan.kind != TimespanKind::Duration) {
            throw Mars2GribModelException("AIFSStandardSingleLoop requires a duration-valued timespan", input.to_json(),
                                          Here());
        }

        if (!input.timespan.duration.has_value()) {
            throw Mars2GribModelException("AIFSStandardSingleLoop duration-valued timespan must contain a duration",
                                          input.to_json(), Here());
        }

        if (input.timeIncrement.has_value()) {
            throw Mars2GribModelException("AIFS statistics require timeIncrementInSeconds to be missing",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AIFSStandardSingleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration timeRange = *input.timespan.duration;

        if (!compareTimeDuration(timeRange, *outerTimeRange.timeRange)) {
            throw Mars2GribModelException(
                "AIFSStandardSingleLoop timespan duration does not match the resolved outer time range",
                input.to_json(), Here());
        }

        // This case carries the normalized innermost statistical processing
        // directly into the one canonical window.
        const auto typeOfStatisticalProcessing = input.innerMostTypeOfStatisticalProcessing;

        // Pure AIFS single-loop products always encode missing increment-kind semantics.
        const auto typeOfTimeIncrement = missingTypeOfTimeIncrement();

        // The canonical range is exactly the normalized duration-valued timespan.
        const auto canonicalTimeRange = timeRange;

        // Pure AIFS single-loop products always encode a missing increment value.
        const auto timeIncrement = missingIncrement();

        ProductTimeSpecWindow window{typeOfStatisticalProcessing, typeOfTimeIncrement, canonicalTimeRange,
                                     timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_AIFSStandardSingleLoop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved AIFSStandardSingleLoop shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `AIFSStandardSingleLoop`;
 * - the originating input still satisfies the case semantics;
 * - the stage-1 outer time range matches the direct duration-valued `timespan`;
 * - the resolved shape contains exactly one canonical window;
 * - that window matches the locally recomputed AIFS single-loop semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the AIFSStandardSingleLoop case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_AIFSStandardSingleLoop_Shape(
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
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::tables::TypeOfTimeIntervals;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::AIFSStandardSingleLoop) {
            throw Mars2GribModelException("Shape classification mismatch: expected AIFSStandardSingleLoop",
                                          input.to_json(), Here());
        }

        const bool isAifs     = input.regime == SimulationRegime::AIFS;
        const bool isForecast = input.simulationType == SimulationType::Forecast;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics               = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal                          = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isNotSynoptic                          = !input.isSynoptic;
        const bool sourceIncrementIsMissing               = !input.timeIncrement.has_value();
        const bool hasDurationTimespan                    = input.timespan.kind == TimespanKind::Duration;
        const bool hasDurationValue                       = input.timespan.duration.has_value();
        const bool hasNoStattypeBlocks                    = input.stattype.empty();
        const bool requiresFakeDoubleLoop                 = input.requiresFakeDoubleLoopSingleLoopRepresentation;
        const bool requiresFakeSecondLoop                 = input.requiresFakeSingleLoopDoubleLoopRepresentation;
        const bool doesNotRequireFakeDoubleLoop           = !requiresFakeDoubleLoop;
        const bool doesNotRequireFakeSingleLoopDoubleLoop = !requiresFakeSecondLoop;

        if (!isAifs || !isForecast || !isNotSeasonal || !isNotSynoptic || !sourceIncrementIsMissing ||
            !hasDurationTimespan || !hasDurationValue || !hasNoStattypeBlocks || !doesNotRequireFakeDoubleLoop ||
            !doesNotRequireFakeSingleLoopDoubleLoop) {
            throw Mars2GribModelException("AIFSStandardSingleLoop input semantics are not satisfied", input.to_json(),
                                          Here());
        }

        if (domain.isSynoptic) {
            throw Mars2GribModelException("AIFSStandardSingleLoop shape must not be paired with a synoptic domain",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AIFSStandardSingleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration expectedTimeRange = *input.timespan.duration;

        if (!compareTimeDuration(*outerTimeRange.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException(
                "AIFSStandardSingleLoop outer time range does not match the direct timespan duration", input.to_json(),
                Here());
        }

        if (shape.values.size() != 1) {
            throw Mars2GribModelException("AIFSStandardSingleLoop shape must contain exactly one window",
                                          input.to_json(), Here());
        }

        const ProductTimeSpecWindow& window = shape.values.front();

        if (window.typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "AIFSStandardSingleLoop window statistical processing does not match the innermost input processing",
                input.to_json(), Here());
        }

        if (window.typeOfTimeIncrement != TypeOfTimeIntervals::Missing) {
            throw Mars2GribModelException("AIFSStandardSingleLoop window typeOfTimeIncrement must be missing",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("AIFSStandardSingleLoop window timeRange is inconsistent", input.to_json(),
                                          Here());
        }

        if (!compareTimeDuration(window.timeIncrement, missingIncrement())) {
            throw Mars2GribModelException("AIFSStandardSingleLoop window timeIncrement must be missing",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_AIFSStandardSingleLoop_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

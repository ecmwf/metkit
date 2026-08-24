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
/// @file SeasonalSingleLoop.h
/// @brief Matcher, builders, and checker for the SeasonalSingleLoop shape.
///
/// This header is the authoritative implementation of the `SeasonalSingleLoop`
/// shape case. It keeps recognition, construction, and validation together so
/// that the complete case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the
/// intrinsic one-month outer time range directly in the callback. The final
/// builder constructs the one canonical seasonal window from visible local
/// members and locally resolved increment semantics. The checker validates that
/// the resolved shape remains consistent with both the input semantics and the
/// stage artifacts.
///
/// Every function catches all failures and rethrows a nested
/// `Mars2GribModelException` with the serialized input state.
///
/// @ingroup mars2grib_product_time_spec_shapes
///
#pragma once

#include <sstream>

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/TimeIncrement.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeUtils.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Return true only when input matches the SeasonalSingleLoop shape.
 *
 * - the product uses the seasonal class/stream discriminator:
 *   - `marsClass` is `od`, `rd`, or `c3`;
 *   - `marsStream` is `sfmd` or `shmd`;
 * - the normalized input also carries seasonal lead semantics:
 *   - `step` is absent;
 *   - `fcmonth` is present;
 * - the product is forecast;
 * - the product is not synoptic;
 * - `timespan` is explicitly `none`, or it is missing and the configured
 *   statistical-product rule allows that source representation;
 * - exactly one outer `stattype` block is present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_SeasonalSingleLoop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasSeasonalClass  = input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3";
        const bool hasSeasonalStream = input.marsStream == "sfmd" || input.marsStream == "shmd";
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isForecast               = input.simulationType == SimulationType::Forecast;
        const bool isNotSynoptic            = !input.isSynoptic;
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) ||
            timespanIsMissingAndAllowed(input, input.allowMissingTimespanForStatisticalProduct);
        const bool hasExactlyOneStattypeBlock = input.stattype.size() == 1;

        return hasSeasonalClass && hasSeasonalStream && hasSeasonalLeadSemantics && isForecast && isNotSynoptic &&
               hasAcceptedTimespanRepresentation && hasExactlyOneStattypeBlock;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalSingleLoop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the SeasonalSingleLoop shape.
 *
 * In this case:
 * - the seasonal single-loop window range is intrinsic;
 * - the canonical outer time range is exactly one calendar month;
 * - the source `stattype` block is redundant and must agree with the
 *   innermost statistical processing.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_SeasonalSingleLoop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::enum2name_TypeOfStatisticalProcessing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)classification;

        if (input.stattype.size() != 1) {
            throw Mars2GribModelException("SeasonalSingleLoop requires exactly one stattype block", input.to_json(),
                                          Here());
        }

        const auto& stattype = input.stattype.front();
        const bool processingTypesMatch =
            stattype.typeOfStatisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            std::ostringstream os;
            os << "SeasonalSingleLoop stattype processing must match the innermost processing: stattype="
               << enum2name_TypeOfStatisticalProcessing_or_throw(stattype.typeOfStatisticalProcessing) << ", innermost="
               << enum2name_TypeOfStatisticalProcessing_or_throw(input.innerMostTypeOfStatisticalProcessing);
            throw Mars2GribModelException(os.str(), input.to_json(), Here());
        }

        const TimeDuration timeRange = oneMonth();

        if (!compareTimeDuration(stattype.timeRange, timeRange)) {
            throw Mars2GribModelException("SeasonalSingleLoop stattype range must be one calendar month",
                                          input.to_json(), Here());
        }

        const auto availability = ProductTimeSpecOuterTimeRangeAvailability::Available;

        return ProductTimeSpecOuterTimeRange{availability, timeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_SeasonalSingleLoop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the canonical seasonal single-loop window.
 *
 * In this case:
 * - the unique `stattype` block is redundant and is checked against the
 *   intrinsic one-month seasonal range and the innermost processing type;
 * - the statistical processing type is the innermost normalized processing;
 * - the canonical time range is one calendar month;
 * - increment resolution follows the normal single-loop structure;
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
inline ProductTimeSpecShape build_SeasonalSingleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::deduceDefaultTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::enum2name_TypeOfStatisticalProcessing_or_throw;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;

        if (input.stattype.size() != 1) {
            throw Mars2GribModelException("SeasonalSingleLoop requires exactly one stattype block", input.to_json(),
                                          Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("SeasonalSingleLoop requires an available outer time range", input.to_json(),
                                          Here());
        }

        const auto& stattype = input.stattype.front();
        const bool processingTypesMatch =
            stattype.typeOfStatisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            std::ostringstream os;
            os << "SeasonalSingleLoop stattype processing must match the innermost processing: stattype="
               << enum2name_TypeOfStatisticalProcessing_or_throw(stattype.typeOfStatisticalProcessing) << ", innermost="
               << enum2name_TypeOfStatisticalProcessing_or_throw(input.innerMostTypeOfStatisticalProcessing);
            throw Mars2GribModelException(os.str(), input.to_json(), Here());
        }

        const TimeDuration timeRange = oneMonth();

        if (!compareTimeDuration(*outerTimeRange.timeRange, timeRange)) {
            throw Mars2GribModelException("SeasonalSingleLoop outer time range must be one calendar month",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(stattype.timeRange, timeRange)) {
            throw Mars2GribModelException("SeasonalSingleLoop stattype range must be one calendar month",
                                          input.to_json(), Here());
        }

        ResolvedInnerIncrement resolvedIncrement{};

        if (input.timeIncrement.has_value()) {
            const long incrementInSeconds = convertToSeconds(*input.timeIncrement);

            if (incrementInSeconds <= 0) {
                throw Mars2GribModelException("Explicit timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            resolvedIncrement.timeIncrement       = TimeDuration{incrementInSeconds, TimeUnit::Second};
            resolvedIncrement.typeOfTimeIncrement = typeOfTimeIncrementForWindow(input, false, true, timeRange);
        }
        else if (!input.allowDefaultTimeIncrement) {
            resolvedIncrement.timeIncrement       = missingIncrement();
            resolvedIncrement.typeOfTimeIncrement = missingTypeOfTimeIncrement();
        }
        else {
            throw Mars2GribModelException("Default time-increment deduction for SeasonalSingleLoop is not implemented",
                                          input.to_json(), Here());

            const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, timeRange);

            if (defaultIncrementInSeconds <= 0) {
                throw Mars2GribModelException("Defaulted timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            resolvedIncrement.timeIncrement       = TimeDuration{defaultIncrementInSeconds, TimeUnit::Second};
            resolvedIncrement.typeOfTimeIncrement = typeOfTimeIncrementForWindow(input, false, true, timeRange);
        }

        // The redundant stattype block must agree with the real innermost seasonal processing.
        const auto typeOfStatisticalProcessing = input.innerMostTypeOfStatisticalProcessing;

        // The increment kind follows the locally resolved single-loop rules.
        const auto typeOfTimeIncrement = resolvedIncrement.typeOfTimeIncrement;

        // The canonical seasonal single-loop range is intrinsically one month.
        const auto canonicalTimeRange = timeRange;

        // The increment is the explicit, missing, or defaulted value resolved above.
        const auto timeIncrement = resolvedIncrement.timeIncrement;

        ProductTimeSpecWindow window{typeOfStatisticalProcessing, typeOfTimeIncrement, canonicalTimeRange,
                                     timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_SeasonalSingleLoop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved SeasonalSingleLoop shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `SeasonalSingleLoop`;
 * - the originating input still satisfies the seasonal class/stream and lead semantics;
 * - the stage-1 outer time range is available and equals one calendar month;
 * - the resolved shape contains exactly one canonical window;
 * - the redundant `stattype` block matches the one-month range and innermost processing;
 * - that window matches the locally recomputed seasonal single-loop semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the SeasonalSingleLoop case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_SeasonalSingleLoop_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const ProductTimeSpecShape& shape) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::deduceDefaultTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::ResolvedInnerIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::typeOfTimeIncrementForWindow;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecWindow;
    using metkit::mars2grib::backend::tables::enum2name_TypeOfStatisticalProcessing_or_throw;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;
    using metkit::mars2grib::utils::time_arithmetic::oneMonth;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::SeasonalSingleLoop) {
            throw Mars2GribModelException("Shape classification mismatch: expected SeasonalSingleLoop", input.to_json(),
                                          Here());
        }

        const bool hasSeasonalClass  = input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3";
        const bool hasSeasonalStream = input.marsStream == "sfmd" || input.marsStream == "shmd";
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isForecast               = input.simulationType == SimulationType::Forecast;
        const bool isNotSynoptic            = !input.isSynoptic;
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) ||
            timespanIsMissingAndAllowed(input, input.allowMissingTimespanForStatisticalProduct);
        const bool hasExactlyOneStattypeBlock = input.stattype.size() == 1;

        if (!hasSeasonalClass || !hasSeasonalStream || !hasSeasonalLeadSemantics || !isForecast || !isNotSynoptic ||
            !hasAcceptedTimespanRepresentation || !hasExactlyOneStattypeBlock) {
            throw Mars2GribModelException("SeasonalSingleLoop input semantics are not satisfied", input.to_json(),
                                          Here());
        }

        if (domain.isSynoptic) {
            throw Mars2GribModelException("SeasonalSingleLoop shape must not be paired with a synoptic domain",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("SeasonalSingleLoop requires an available outer time range", input.to_json(),
                                          Here());
        }

        const auto& stattype = input.stattype.front();
        const bool processingTypesMatch =
            stattype.typeOfStatisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            std::ostringstream os;
            os << "SeasonalSingleLoop stattype processing must match the innermost processing: stattype="
               << enum2name_TypeOfStatisticalProcessing_or_throw(stattype.typeOfStatisticalProcessing) << ", innermost="
               << enum2name_TypeOfStatisticalProcessing_or_throw(input.innerMostTypeOfStatisticalProcessing);
            throw Mars2GribModelException(os.str(), input.to_json(), Here());
        }

        const TimeDuration expectedTimeRange = oneMonth();

        if (!compareTimeDuration(*outerTimeRange.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("SeasonalSingleLoop outer time range must be one calendar month",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(stattype.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("SeasonalSingleLoop stattype range must be one calendar month",
                                          input.to_json(), Here());
        }

        if (shape.values.size() != 1) {
            throw Mars2GribModelException("SeasonalSingleLoop shape must contain exactly one window", input.to_json(),
                                          Here());
        }

        ResolvedInnerIncrement expectedIncrement{};

        if (input.timeIncrement.has_value()) {
            const long incrementInSeconds = convertToSeconds(*input.timeIncrement);

            if (incrementInSeconds <= 0) {
                throw Mars2GribModelException("Explicit timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            expectedIncrement.timeIncrement       = TimeDuration{incrementInSeconds, TimeUnit::Second};
            expectedIncrement.typeOfTimeIncrement = typeOfTimeIncrementForWindow(input, false, true, expectedTimeRange);
        }
        else if (!input.allowDefaultTimeIncrement) {
            expectedIncrement.timeIncrement       = missingIncrement();
            expectedIncrement.typeOfTimeIncrement = missingTypeOfTimeIncrement();
        }
        else {
            throw Mars2GribModelException("Default time-increment deduction for SeasonalSingleLoop is not implemented",
                                          input.to_json(), Here());

            const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, expectedTimeRange);

            if (defaultIncrementInSeconds <= 0) {
                throw Mars2GribModelException("Defaulted timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            expectedIncrement.timeIncrement       = TimeDuration{defaultIncrementInSeconds, TimeUnit::Second};
            expectedIncrement.typeOfTimeIncrement = typeOfTimeIncrementForWindow(input, false, true, expectedTimeRange);
        }

        const ProductTimeSpecWindow& window = shape.values.front();

        if (window.typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "SeasonalSingleLoop window statistical processing does not match the innermost input processing",
                input.to_json(), Here());
        }

        if (window.typeOfTimeIncrement != expectedIncrement.typeOfTimeIncrement) {
            throw Mars2GribModelException("SeasonalSingleLoop window typeOfTimeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeRange, expectedTimeRange)) {
            throw Mars2GribModelException("SeasonalSingleLoop window timeRange is inconsistent", input.to_json(),
                                          Here());
        }

        if (!compareTimeDuration(window.timeIncrement, expectedIncrement.timeIncrement)) {
            throw Mars2GribModelException("SeasonalSingleLoop window timeIncrement is inconsistent", input.to_json(),
                                          Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_SeasonalSingleLoop_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

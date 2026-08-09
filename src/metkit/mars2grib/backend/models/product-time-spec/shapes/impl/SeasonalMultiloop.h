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
/// @file SeasonalMultiloop.h
/// @brief Matcher, builders, and checker for the SeasonalMultiloop shape.
///
/// This header is the authoritative implementation of the `SeasonalMultiloop`
/// shape case. It keeps recognition, construction, and validation together so
/// that the complete case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the outer
/// seasonal loop range directly from the outermost `stattype` block. The final
/// builder constructs all canonical windows from visible local members and
/// locally resolved innermost increment semantics. The checker validates that
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
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail {

/**
 * @brief Return true only when input matches the SeasonalMultiloop shape.
 *
 * - the product uses the seasonal class/stream discriminator:
 *   - `marsClass` is `od`, `rd`, or `c3`;
 *   - `marsStream` is `sfmd` or `shmd`;
 * - the normalized input also carries seasonal lead semantics:
 *   - `step` is absent;
 *   - `fcmonth` is present;
 * - the product is forecast;
 * - the product is not synoptic;
 * - `timespan` is duration-valued and contains the innermost range;
 * - one or more outer `stattype` blocks are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_SeasonalMultiloop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasSeasonalClass    = input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3";
        const bool hasSeasonalStream   = input.marsStream == "sfmd" || input.marsStream == "shmd";
        const bool hasNoStep           = !input.step.has_value();
        const bool hasFcmonth          = input.marsFcmonth.has_value();
        const bool isForecast          = input.simulationType == SimulationType::Forecast;
        const bool isNotSynoptic       = !input.isSynoptic;
        const bool hasDurationTimespan = input.timespan.kind == TimespanKind::Duration;
        const bool hasTimespanDuration = input.timespan.duration.has_value();
        const bool hasOuterStattypeBlocks = !input.stattype.empty();

        return hasSeasonalClass && hasSeasonalStream && hasNoStep && hasFcmonth && isForecast && isNotSynoptic &&
               hasDurationTimespan && hasTimespanDuration && hasOuterStattypeBlocks;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalMultiloop_Shape`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the SeasonalMultiloop shape.
 *
 * In this case:
 * - at least one outer `stattype` block must be present;
 * - the outermost `stattype` block is the seasonal monthly loop;
 * - the outermost `stattype` range must therefore be exactly one month;
 * - any later monthly `stattype` block is treated as an invalid normalized state.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_SeasonalMultiloop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        if (input.stattype.empty()) {
            throw Mars2GribModelException("SeasonalMultiloop requires at least one stattype block", input.to_json(),
                                          Here());
        }

        const TimeDuration outerTimeRange = input.stattype.front().timeRange;
        const bool outerRangeIsMonthly    = outerTimeRange.unit == TimeUnit::Month && outerTimeRange.length == 1;

        if (!outerRangeIsMonthly) {
            throw Mars2GribModelException("SeasonalMultiloop outermost stattype range must be one calendar month",
                                          input.to_json(), Here());
        }

        for (std::size_t i = 1; i < input.stattype.size(); ++i) {
            const auto& timeRange = input.stattype[i].timeRange;
            const bool isMonthly  = timeRange.unit == TimeUnit::Month && timeRange.length == 1;

            if (isMonthly) {
                throw Mars2GribModelException("SeasonalMultiloop must not contain a second monthly stattype block",
                                              input.to_json(), Here());
            }
        }

        const auto availability = ProductTimeSpecOuterTimeRangeAvailability::Available;

        return ProductTimeSpecOuterTimeRange{availability, outerTimeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_SeasonalMultiloop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct all canonical seasonal multiloop windows in outermost-to-innermost order.
 *
 * In this case:
 * - the outermost `stattype` block is the seasonal monthly loop;
 * - any additional `stattype` blocks are inner outer-loops and must not be monthly;
 * - the innermost range is the normalized duration-valued `timespan` itself;
 * - each outer window increment equals the next inner window range;
 * - the innermost increment is resolved locally from explicit, missing, or
 *   defaulted input rules.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @return Constructed ProductTimeSpec shape for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecShape build_SeasonalMultiloop_ShapeWindows(
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
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;

        if (input.timespan.kind != TimespanKind::Duration) {
            throw Mars2GribModelException("SeasonalMultiloop requires a duration-valued timespan", input.to_json(),
                                          Here());
        }

        if (!input.timespan.duration.has_value()) {
            throw Mars2GribModelException("SeasonalMultiloop duration-valued timespan must contain a duration",
                                          input.to_json(), Here());
        }

        if (input.stattype.empty()) {
            throw Mars2GribModelException("SeasonalMultiloop requires at least one stattype block", input.to_json(),
                                          Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("SeasonalMultiloop requires an available outer time range", input.to_json(),
                                          Here());
        }

        const TimeDuration expectedOuterTimeRange = input.stattype.front().timeRange;
        const bool outerRangeIsMonthly =
            expectedOuterTimeRange.unit == TimeUnit::Month && expectedOuterTimeRange.length == 1;

        if (!outerRangeIsMonthly) {
            throw Mars2GribModelException("SeasonalMultiloop outermost stattype range must be one calendar month",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(*outerTimeRange.timeRange, expectedOuterTimeRange)) {
            throw Mars2GribModelException(
                "SeasonalMultiloop outer time range does not match the outermost stattype range", input.to_json(),
                Here());
        }

        for (std::size_t i = 1; i < input.stattype.size(); ++i) {
            const auto& timeRange = input.stattype[i].timeRange;
            const bool isMonthly  = timeRange.unit == TimeUnit::Month && timeRange.length == 1;

            if (isMonthly) {
                throw Mars2GribModelException("SeasonalMultiloop must not contain a second monthly stattype block",
                                              input.to_json(), Here());
            }
        }

        std::vector<ProductTimeSpecWindow> windows;
        windows.reserve(input.stattype.size() + 1);

        for (std::size_t i = 0; i < input.stattype.size(); ++i) {
            const auto& stattypeBlock = input.stattype[i];
            const auto& timeRange     = stattypeBlock.timeRange;

            // Each outer loop preserves the source statistical processing declared by its stattype block.
            const auto typeOfStatisticalProcessing = stattypeBlock.typeOfStatisticalProcessing;

            // Outer-loop increment-kind semantics follow the multi-loop outer-window rules.
            const auto typeOfTimeIncrement = typeOfTimeIncrementForWindow(input, true, false, timeRange);

            // Each outer loop range comes directly from its stattype block.
            const auto outerWindowTimeRange = timeRange;

            // The increment is filled later from the next inner window range.
            const auto timeIncrement = missingIncrement();

            windows.push_back(ProductTimeSpecWindow{typeOfStatisticalProcessing, typeOfTimeIncrement,
                                                    outerWindowTimeRange, timeIncrement});
        }

        const TimeDuration innermostTimeRange = *input.timespan.duration;

        ResolvedInnerIncrement resolvedInnermostIncrement{};

        if (input.timeIncrement.has_value()) {
            const long incrementInSeconds = convertToSeconds(*input.timeIncrement);

            if (incrementInSeconds <= 0) {
                throw Mars2GribModelException("Explicit timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (innermostTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(innermostTimeRange);

                if (incrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            resolvedInnermostIncrement.timeIncrement = TimeDuration{incrementInSeconds, TimeUnit::Second};
            resolvedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, innermostTimeRange);
        }
        else if (!input.allowDefaultTimeIncrement) {
            resolvedInnermostIncrement.timeIncrement       = missingIncrement();
            resolvedInnermostIncrement.typeOfTimeIncrement = missingTypeOfTimeIncrement();
        }
        else {
            throw Mars2GribModelException("Default time-increment deduction for SeasonalMultiloop is not implemented",
                                          input.to_json(), Here());

            const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, innermostTimeRange);

            if (defaultIncrementInSeconds <= 0) {
                throw Mars2GribModelException("Defaulted timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (innermostTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(innermostTimeRange);

                if (defaultIncrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            resolvedInnermostIncrement.timeIncrement = TimeDuration{defaultIncrementInSeconds, TimeUnit::Second};
            resolvedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, innermostTimeRange);
        }

        // The innermost loop uses the real normalized innermost processing and the
        // duration-valued timespan supplied by the input.
        windows.push_back(ProductTimeSpecWindow{input.innerMostTypeOfStatisticalProcessing,
                                                resolvedInnermostIncrement.typeOfTimeIncrement, innermostTimeRange,
                                                resolvedInnermostIncrement.timeIncrement});

        for (std::size_t windowIndex = 0; windowIndex + 1 < windows.size(); ++windowIndex) {
            windows[windowIndex].timeIncrement = windows[windowIndex + 1].timeRange;
        }

        return ProductTimeSpecShape{windows};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_SeasonalMultiloop_ShapeWindows`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved SeasonalMultiloop shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `SeasonalMultiloop`;
 * - the originating input still satisfies the seasonal class/stream and lead semantics;
 * - the stage-1 outer time range matches the outermost monthly stattype block;
 * - the resolved shape contains one window per stattype block plus one innermost window;
 * - each outer window increment equals the next inner window range;
 * - the innermost window matches the locally recomputed seasonal multiloop semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the SeasonalMultiloop case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_SeasonalMultiloop_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const ProductTimeSpecShape& shape) {
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
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;
    using metkit::mars2grib::utils::time_arithmetic::convertToSeconds;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::SeasonalMultiloop) {
            throw Mars2GribModelException("Shape classification mismatch: expected SeasonalMultiloop", input.to_json(),
                                          Here());
        }

        const bool hasSeasonalClass    = input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3";
        const bool hasSeasonalStream   = input.marsStream == "sfmd" || input.marsStream == "shmd";
        const bool hasNoStep           = !input.step.has_value();
        const bool hasFcmonth          = input.marsFcmonth.has_value();
        const bool isForecast          = input.simulationType == SimulationType::Forecast;
        const bool isNotSynoptic       = !input.isSynoptic;
        const bool hasDurationTimespan = input.timespan.kind == TimespanKind::Duration;
        const bool hasTimespanDuration = input.timespan.duration.has_value();
        const bool hasOuterStattypeBlocks = !input.stattype.empty();

        if (!hasSeasonalClass || !hasSeasonalStream || !hasNoStep || !hasFcmonth || !isForecast || !isNotSynoptic ||
            !hasDurationTimespan || !hasTimespanDuration || !hasOuterStattypeBlocks) {
            throw Mars2GribModelException("SeasonalMultiloop input semantics are not satisfied", input.to_json(),
                                          Here());
        }

        if (domain.isSynoptic) {
            throw Mars2GribModelException("SeasonalMultiloop shape must not be paired with a synoptic domain",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("SeasonalMultiloop requires an available outer time range", input.to_json(),
                                          Here());
        }

        const TimeDuration expectedOuterTimeRange = input.stattype.front().timeRange;
        const bool outerRangeIsMonthly =
            expectedOuterTimeRange.unit == TimeUnit::Month && expectedOuterTimeRange.length == 1;

        if (!outerRangeIsMonthly) {
            throw Mars2GribModelException("SeasonalMultiloop outermost stattype range must be one calendar month",
                                          input.to_json(), Here());
        }

        for (std::size_t i = 1; i < input.stattype.size(); ++i) {
            const auto& timeRange = input.stattype[i].timeRange;
            const bool isMonthly  = timeRange.unit == TimeUnit::Month && timeRange.length == 1;

            if (isMonthly) {
                throw Mars2GribModelException("SeasonalMultiloop must not contain a second monthly stattype block",
                                              input.to_json(), Here());
            }
        }

        if (!compareTimeDuration(*outerTimeRange.timeRange, expectedOuterTimeRange)) {
            throw Mars2GribModelException(
                "SeasonalMultiloop outer time range does not match the outermost stattype range", input.to_json(),
                Here());
        }

        if (shape.values.size() != input.stattype.size() + 1) {
            throw Mars2GribModelException(
                "SeasonalMultiloop shape must contain one window per stattype block plus one innermost window",
                input.to_json(), Here());
        }

        const TimeDuration innermostTimeRange = *input.timespan.duration;

        ResolvedInnerIncrement expectedInnermostIncrement{};

        if (input.timeIncrement.has_value()) {
            const long incrementInSeconds = convertToSeconds(*input.timeIncrement);

            if (incrementInSeconds <= 0) {
                throw Mars2GribModelException("Explicit timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (innermostTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(innermostTimeRange);

                if (incrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            expectedInnermostIncrement.timeIncrement = TimeDuration{incrementInSeconds, TimeUnit::Second};
            expectedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, innermostTimeRange);
        }
        else if (!input.allowDefaultTimeIncrement) {
            expectedInnermostIncrement.timeIncrement       = missingIncrement();
            expectedInnermostIncrement.typeOfTimeIncrement = missingTypeOfTimeIncrement();
        }
        else {
            throw Mars2GribModelException("Default time-increment deduction for SeasonalMultiloop is not implemented",
                                          input.to_json(), Here());

            const long defaultIncrementInSeconds = deduceDefaultTimeIncrement(input, innermostTimeRange);

            if (defaultIncrementInSeconds <= 0) {
                throw Mars2GribModelException("Defaulted timeIncrementInSeconds must be positive", input.to_json(),
                                              Here());
            }

            if (innermostTimeRange.unit != TimeUnit::Month) {
                const long timeRangeInSeconds = convertToSeconds(innermostTimeRange);

                if (defaultIncrementInSeconds > timeRangeInSeconds) {
                    throw Mars2GribModelException("timeIncrementInSeconds exceeds the innermost time range",
                                                  input.to_json(), Here());
                }
            }

            expectedInnermostIncrement.timeIncrement = TimeDuration{defaultIncrementInSeconds, TimeUnit::Second};
            expectedInnermostIncrement.typeOfTimeIncrement =
                typeOfTimeIncrementForWindow(input, true, true, innermostTimeRange);
        }

        for (std::size_t i = 0; i < input.stattype.size(); ++i) {
            const ProductTimeSpecWindow& window = shape.values[i];
            const auto& stattypeBlock           = input.stattype[i];

            if (window.typeOfStatisticalProcessing != stattypeBlock.typeOfStatisticalProcessing) {
                throw Mars2GribModelException("SeasonalMultiloop outer window statistical processing is inconsistent",
                                              input.to_json(), Here());
            }

            if (window.typeOfTimeIncrement !=
                typeOfTimeIncrementForWindow(input, true, false, stattypeBlock.timeRange)) {
                throw Mars2GribModelException("SeasonalMultiloop outer window typeOfTimeIncrement is inconsistent",
                                              input.to_json(), Here());
            }

            if (!compareTimeDuration(window.timeRange, stattypeBlock.timeRange)) {
                throw Mars2GribModelException("SeasonalMultiloop outer window timeRange is inconsistent",
                                              input.to_json(), Here());
            }

            if (!compareTimeDuration(window.timeIncrement, shape.values[i + 1].timeRange)) {
                throw Mars2GribModelException(
                    "SeasonalMultiloop outer window timeIncrement must equal the next inner window range",
                    input.to_json(), Here());
            }
        }

        const ProductTimeSpecWindow& innermostWindow = shape.values.back();

        if (innermostWindow.typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "SeasonalMultiloop innermost window statistical processing does not match the innermost input "
                "processing",
                input.to_json(), Here());
        }

        if (innermostWindow.typeOfTimeIncrement != expectedInnermostIncrement.typeOfTimeIncrement) {
            throw Mars2GribModelException("SeasonalMultiloop innermost window typeOfTimeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(innermostWindow.timeRange, innermostTimeRange)) {
            throw Mars2GribModelException("SeasonalMultiloop innermost window timeRange is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(innermostWindow.timeIncrement, expectedInnermostIncrement.timeIncrement)) {
            throw Mars2GribModelException("SeasonalMultiloop innermost window timeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_SeasonalMultiloop_Shape`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

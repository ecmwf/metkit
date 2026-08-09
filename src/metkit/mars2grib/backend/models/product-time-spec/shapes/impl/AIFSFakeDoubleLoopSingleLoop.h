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
/// @file AIFSFakeDoubleLoopSingleLoop.h
/// @brief Matcher, builders, and checker for the AIFSFakeDoubleLoopSingleLoop shape.
///
/// This header is the authoritative implementation of the
/// `AIFSFakeDoubleLoopSingleLoop` shape case. It keeps recognition,
/// construction, and validation together so that the complete case can be
/// reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and
/// returns their explicit conjunction. The stage-1 builder constructs the
/// canonical outer time range directly from the unique `stattype` block. The
/// final builder constructs the one canonical AIFS statistical window from
/// visible local members. The checker validates that the resolved shape remains
/// consistent with both the input semantics and the stage artifacts.
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
 * @brief Return true only when input matches the AIFSFakeDoubleLoopSingleLoop shape.
 *
 * - the regime is AIFS;
 * - the product is forecast;
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - the product is not synoptic;
 * - the source increment is missing;
 * - `timespan` is explicitly `none`, or it is missing and the configured
 *   statistical-product rule allows that source representation;
 * - exactly one outer `stattype` block is present;
 * - the product requires the fake-double-loop compatibility case.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the shape matcher fails unexpectedly.
 */
inline bool match_AIFSFakeDoubleLoopSingleLoop_Shape(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
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
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) ||
            timespanIsMissingAndAllowed(input, input.allowMissingTimespanForStatisticalProduct);
        const bool hasExactlyOneStattypeBlock = input.stattype.size() == 1;
        const bool requiresFakeDoubleLoop     = input.requiresFakeDoubleLoopSingleLoopRepresentation;

        return isAifs && isForecast && isNotSeasonal && isNotSynoptic && sourceIncrementIsMissing &&
               hasAcceptedTimespanRepresentation && hasExactlyOneStattypeBlock && requiresFakeDoubleLoop;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `match_AIFSFakeDoubleLoopSingleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief Construct the stage-1 outer range for the AIFSFakeDoubleLoopSingleLoop shape.
 *
 * In this case:
 * - exactly one outer `stattype` block must be present;
 * - that block must use the same processing type as the innermost processing;
 * - that block range itself is the canonical outer time range;
 * - the original normalized duration unit is preserved.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed stage-1 outer time range for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecOuterTimeRange build_AIFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRange;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::enum2name_TypeOfStatisticalProcessing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        if (input.timeIncrement.has_value()) {
            throw Mars2GribModelException("AIFS statistics require timeIncrementInSeconds to be missing",
                                          input.to_json(), Here());
        }

        if (input.stattype.size() != 1) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop requires exactly one stattype block",
                                          input.to_json(), Here());
        }

        const auto& stattype = input.stattype.front();
        const bool processingTypesMatch =
            stattype.typeOfStatisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            std::ostringstream os;
            os << "AIFSFakeDoubleLoopSingleLoop stattype processing must match the innermost processing: stattype="
               << enum2name_TypeOfStatisticalProcessing_or_throw(stattype.typeOfStatisticalProcessing) << ", innermost="
               << enum2name_TypeOfStatisticalProcessing_or_throw(input.innerMostTypeOfStatisticalProcessing);
            throw Mars2GribModelException(os.str(), input.to_json(), Here());
        }

        const TimeDuration timeRange = stattype.timeRange;
        const auto availability      = ProductTimeSpecOuterTimeRangeAvailability::Available;

        return ProductTimeSpecOuterTimeRange{availability, timeRange};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_AIFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the canonical AIFS fake-double-loop single window.
 *
 * In this case:
 * - the unique outer `stattype` block is promoted to the canonical window;
 * - the statistical processing type comes from that unique block;
 * - the canonical range is that unique block range;
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
inline ProductTimeSpecShape build_AIFSFakeDoubleLoopSingleLoop_ShapeWindows(
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::enum2name_TypeOfStatisticalProcessing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;

    try {
        (void)classification;
        (void)anchor;
        (void)domain;

        if (input.timeIncrement.has_value()) {
            throw Mars2GribModelException("AIFS statistics require timeIncrementInSeconds to be missing",
                                          input.to_json(), Here());
        }

        if (input.stattype.size() != 1) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop requires exactly one stattype block",
                                          input.to_json(), Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const auto& stattype = input.stattype.front();
        const bool processingTypesMatch =
            stattype.typeOfStatisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            std::ostringstream os;
            os << "AIFSFakeDoubleLoopSingleLoop stattype processing must match the innermost processing: stattype="
               << enum2name_TypeOfStatisticalProcessing_or_throw(stattype.typeOfStatisticalProcessing) << ", innermost="
               << enum2name_TypeOfStatisticalProcessing_or_throw(input.innerMostTypeOfStatisticalProcessing);
            throw Mars2GribModelException(os.str(), input.to_json(), Here());
        }

        const auto timeRange = stattype.timeRange;

        if (!compareTimeDuration(timeRange, *outerTimeRange.timeRange)) {
            throw Mars2GribModelException(
                "AIFSFakeDoubleLoopSingleLoop stattype range does not match the resolved outer time range",
                input.to_json(), Here());
        }

        // This case promotes the unique stattype block directly into the one canonical window.
        const auto typeOfStatisticalProcessing = stattype.typeOfStatisticalProcessing;

        // Pure AIFS single-loop products always encode missing increment-kind semantics.
        const auto typeOfTimeIncrement = missingTypeOfTimeIncrement();

        // The canonical range is the unique stattype block range.
        const auto canonicalTimeRange = timeRange;

        // Pure AIFS single-loop products always encode a missing increment value.
        const auto timeIncrement = missingIncrement();

        ProductTimeSpecWindow window{typeOfStatisticalProcessing, typeOfTimeIncrement, canonicalTimeRange,
                                     timeIncrement};

        return ProductTimeSpecShape{{window}};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `build_AIFSFakeDoubleLoopSingleLoop_ShapeWindows`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved AIFSFakeDoubleLoopSingleLoop shape against its source input and stage artifacts.
 *
 * This checker verifies:
 * - the resolved shape classification is `AIFSFakeDoubleLoopSingleLoop`;
 * - the originating input still satisfies the case semantics;
 * - the stage-1 outer time range matches the unique `stattype` block range;
 * - the resolved shape contains exactly one canonical window;
 * - that window matches the locally recomputed fake-double-loop single-window AIFS semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @param[in] domain Previously constructed ProductTimeSpec domain.
 * @param[in] shape Resolved shape artifact produced by the builder.
 * @return `true` when the shape is valid for the AIFSFakeDoubleLoopSingleLoop case.
 * @throws Mars2GribModelException if the resolved shape is inconsistent with
 *         the input, classification, or case semantics.
 */
inline bool check_AIFSFakeDoubleLoopSingleLoop_Shape(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecClassification& classification,
    const metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecOuterTimeRange& outerTimeRange,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain,
    const ProductTimeSpecShape& shape) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingTypeOfTimeIncrement;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecWindow;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsMissingAndAllowed;
    using metkit::mars2grib::backend::models::product_time_spec::shape::detail::timespanIsNone;
    using metkit::mars2grib::backend::tables::enum2name_TypeOfStatisticalProcessing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::compareTimeDuration;

    try {
        (void)anchor;

        if (classification.shapeType != ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop) {
            throw Mars2GribModelException("Shape classification mismatch: expected AIFSFakeDoubleLoopSingleLoop",
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
        const bool hasAcceptedTimespanRepresentation =
            timespanIsNone(input) ||
            timespanIsMissingAndAllowed(input, input.allowMissingTimespanForStatisticalProduct);
        const bool hasExactlyOneStattypeBlock = input.stattype.size() == 1;
        const bool requiresFakeDoubleLoop     = input.requiresFakeDoubleLoopSingleLoopRepresentation;

        if (!isAifs || !isForecast || !isNotSeasonal || !isNotSynoptic || !sourceIncrementIsMissing ||
            !hasAcceptedTimespanRepresentation || !hasExactlyOneStattypeBlock || !requiresFakeDoubleLoop) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop input semantics are not satisfied",
                                          input.to_json(), Here());
        }

        if (domain.isSynoptic) {
            throw Mars2GribModelException(
                "AIFSFakeDoubleLoopSingleLoop shape must not be paired with a synoptic domain", input.to_json(),
                Here());
        }

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop requires an available outer time range",
                                          input.to_json(), Here());
        }

        const auto& stattype = input.stattype.front();
        const bool processingTypesMatch =
            stattype.typeOfStatisticalProcessing == input.innerMostTypeOfStatisticalProcessing;

        if (!processingTypesMatch) {
            std::ostringstream os;
            os << "AIFSFakeDoubleLoopSingleLoop stattype processing must match the innermost processing: stattype="
               << enum2name_TypeOfStatisticalProcessing_or_throw(stattype.typeOfStatisticalProcessing) << ", innermost="
               << enum2name_TypeOfStatisticalProcessing_or_throw(input.innerMostTypeOfStatisticalProcessing);
            throw Mars2GribModelException(os.str(), input.to_json(), Here());
        }

        if (!compareTimeDuration(*outerTimeRange.timeRange, stattype.timeRange)) {
            throw Mars2GribModelException(
                "AIFSFakeDoubleLoopSingleLoop outer time range does not match the unique stattype range",
                input.to_json(), Here());
        }

        if (shape.values.size() != 1) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop shape must contain exactly one window",
                                          input.to_json(), Here());
        }

        const ProductTimeSpecWindow& window = shape.values.front();

        if (window.typeOfStatisticalProcessing != stattype.typeOfStatisticalProcessing) {
            throw Mars2GribModelException(
                "AIFSFakeDoubleLoopSingleLoop window statistical processing does not match the stattype block",
                input.to_json(), Here());
        }

        if (window.typeOfTimeIncrement != missingTypeOfTimeIncrement()) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop window typeOfTimeIncrement is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeRange, stattype.timeRange)) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop window timeRange is inconsistent",
                                          input.to_json(), Here());
        }

        if (!compareTimeDuration(window.timeIncrement, missingIncrement())) {
            throw Mars2GribModelException("AIFSFakeDoubleLoopSingleLoop window timeIncrement must be missing",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `check_AIFSFakeDoubleLoopSingleLoop_Shape`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape::detail

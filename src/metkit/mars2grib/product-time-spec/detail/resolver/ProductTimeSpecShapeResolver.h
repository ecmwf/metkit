/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


/**
 * @file ProductTimeSpecShapeResolver.h
 * @brief Product-shape classification and shape-artifact construction.
 *
 * This header implements the ProductTimeSpec resolver stage that decides the
 * structural statistical representation of the product and constructs the
 * corresponding `ProductTimeSpecShape` artifact.
 *
 * It owns:
 *
 * - shape classification from normalized `timespan`, `stattype`, `step`, and
 *   `(class, stream)` context;
 * - fake-double-loop representation-policy checks;
 * - shape construction of absolute support placement and innermost range data;
 * - outermost alignment checks for day- and month-based windows;
 * - dispatch to the specialized shape constructors.
 *
 * It does not own:
 *
 * - dictionary access or lexical parsing;
 * - time-anchor or time-increment logic;
 * - final canonical window materialization.
 *
 * The implementation follows `productTimeSpecV3_final.md`, especially
 * Sections 3.3.2, 4.13, 5.10, 5.11, and 5.14.
 */

#pragma once


#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecResolverCommon.h"

namespace metkit::mars2grib::product_time_spec {
namespace resolver_detail {

/**
 * @brief Enforce strict calendar alignment for calendar-derived outer windows.
 *
 * The current supported domain requires:
 *
 * - day-based outer windows to end at midnight;
 * - month-based outer windows to end on day 1 at midnight.
 *
 * Elapsed-hour and elapsed-second outer windows impose no additional calendar
 * alignment requirement here.
 *
 * @param windowEnd Candidate end datetime of the outer support.
 * @param outerRange Outermost canonical range candidate.
 * @throws std::invalid_argument if the calendar alignment rule is violated.
 */
inline void checkOutermostAlignment(const eckit::DateTime& windowEnd,
                                    const ProductTimeDuration& outerRange) {
    if (outerRange.unit == tables::TimeUnit::Day && !isAtMidnight(windowEnd)) {
        throw std::invalid_argument(
            "day-based outermost range requires windowEndDateTime at midnight");
    }
    if (outerRange.unit == tables::TimeUnit::Month &&
        !isOnFirstOfMonthMidnight(windowEnd)) {
        throw std::invalid_argument(
            "month-based outermost range requires windowEndDateTime on day 1 at midnight");
    }
}

/**
 * @brief Construct one shape artifact for a known shape classification.
 *
 * Shape construction resolves:
 *
 * - `windowEndDateTime` from `referenceDateTime + resolvedStep`;
 * - `innerTimeRange` for shapes that own one explicitly;
 * - the absolute support start from the outermost structural time range;
 * - the construction-time zero-length-from-start evidence flag.
 *
 * For fake-double-loop shapes, the single structural range is carried by the
 * sole parsed `stattype` block rather than `innerTimeRange`.
 *
 * @param input Normalized input snapshot.
 * @param anchor Resolved anchor artifact.
 * @param shapeType Already-classified valid shape kind.
 * @return Fully resolved shape artifact.
 * @throws Mars2GribProductTimeSpecException on structural or placement errors.
 */
template <class Input_t>
ProductTimeSpecShape constructShapeCommon(const Input_t& input,
                                          const ProductTimeSpecAnchor& anchor,
                                          ProductTimeSpecShapeKind shapeType) {
    const long step = resolvedStep(input);
    const eckit::DateTime windowEnd =
        addDuration(anchor.referenceDateTime, canonicalElapsedDuration(step));

    ProductTimeSpecShape result;
    result.windowEndDateTime = windowEnd;
    result.stattypeBlocks = input.stattypeBlocks();
    result.shapeType = shapeType;
    result.zeroLengthFromStartWindowByDesign = false;

    switch (shapeType) {
        case ProductTimeSpecShapeKind::Instant:
            result.windowStartDateTime = windowEnd;
            return result;

        case ProductTimeSpecShapeKind::StandardSingleLoop:
        case ProductTimeSpecShapeKind::MultiLoop:
            if (!input.timespanInSeconds()) {
                fail(ProductTimeSpecStage::ShapeConstruction,
                     "duration-valued shape is missing normalized timespanInSeconds",
                     input);
            }
            result.innerTimeRange = canonicalElapsedDuration(*input.timespanInSeconds());
            break;

        case ProductTimeSpecShapeKind::FromStartSingleLoop:
            result.innerTimeRange = canonicalElapsedDuration(step);
            result.zeroLengthFromStartWindowByDesign = (step == 0);
            break;

        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
            break;

        case ProductTimeSpecShapeKind::Count:
            fail(ProductTimeSpecStage::ShapeConstruction,
                 "invalid shape classification sentinel",
                 input);
    }

    ProductTimeDuration outerRange;
    if (shapeType == ProductTimeSpecShapeKind::MultiLoop ||
        shapeType == ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop) {
        if (input.stattypeBlocks().empty()) {
            fail(ProductTimeSpecStage::ShapeConstruction,
                 "shape requires at least one stattype block",
                 input);
        }
        outerRange = input.stattypeBlocks()[0].timeRange;
    } else {
        outerRange = *result.innerTimeRange;
    }

    try {
        checkOutermostAlignment(windowEnd, outerRange);
        result.windowStartDateTime = subtractDuration(windowEnd, outerRange);
    } catch (const std::exception& e) {
        fail(ProductTimeSpecStage::ShapeConstruction,
             std::string("failed to place absolute statistical support: ") + e.what(),
             input);
    }

    if (shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop &&
        result.windowStartDateTime != anchor.referenceDateTime) {
        fail(ProductTimeSpecStage::ShapeConstruction,
             "from-start window start computed from range does not equal referenceDateTime",
             input);
    }

    if (result.windowStartDateTime < anchor.referenceDateTime) {
        fail(ProductTimeSpecStage::ShapeConstruction,
             "statistical support begins before referenceDateTime",
             input);
    }

    return result;
}

/** @brief Construct the `Instant` shape artifact. */
template <class Input_t>
ProductTimeSpecShape constructInstantShape(const Input_t& input,
                                           const ProductTimeSpecAnchor& anchor) {
    return constructShapeCommon(input, anchor, ProductTimeSpecShapeKind::Instant);
}

/** @brief Construct the `StandardSingleLoop` shape artifact. */
template <class Input_t>
ProductTimeSpecShape constructStandardShape(const Input_t& input,
                                            const ProductTimeSpecAnchor& anchor) {
    return constructShapeCommon(input, anchor,
                                ProductTimeSpecShapeKind::StandardSingleLoop);
}

/** @brief Construct the `MultiLoop` shape artifact. */
template <class Input_t>
ProductTimeSpecShape constructMultiShape(const Input_t& input,
                                         const ProductTimeSpecAnchor& anchor) {
    return constructShapeCommon(input, anchor, ProductTimeSpecShapeKind::MultiLoop);
}

/** @brief Construct the `FakeDoubleLoopSingleLoop` shape artifact. */
template <class Input_t>
ProductTimeSpecShape constructFakeDoubleLoopShape(const Input_t& input,
                                                  const ProductTimeSpecAnchor& anchor) {
    return constructShapeCommon(input, anchor,
                                ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop);
}

/** @brief Construct the `FromStartSingleLoop` shape artifact. */
template <class Input_t>
ProductTimeSpecShape constructFromStartShape(const Input_t& input,
                                             const ProductTimeSpecAnchor& anchor) {
    return constructShapeCommon(input, anchor,
                                ProductTimeSpecShapeKind::FromStartSingleLoop);
}

}  // namespace resolver_detail

/**
 * @brief Classify the structural ProductTimeSpec shape.
 *
 * Classification depends on:
 *
 * - normalized `timespanKind`;
 * - parsed `stattype` cardinality;
 * - fake-double-loop representation policy for the active `(class, stream)`;
 * - step-presence and analysis-step consistency rules;
 * - the zero-length from-start policy.
 *
 * @param input Normalized input snapshot.
 * @return Valid `ProductTimeSpecShapeKind` classification.
 * @throws Mars2GribProductTimeSpecException on unsupported or contradictory
 *         structural states.
 */
template <class Input_t>
ProductTimeSpecShapeKind classify_ProductTimeSpecShape_or_throw(const Input_t& input) {
    if (input.marsType() == "an") {
        if (input.stepInSeconds() && *input.stepInSeconds() != 0) {
            resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                  "analysis product has an explicit non-zero `step`",
                                  input);
        }
    } else if (!input.stepInSeconds()) {
        resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                              "missing `step` is allowed only for type='an'",
                              input);
    }

    const auto nStat = input.stattypeBlocks().size();
    const long step = resolver_detail::resolvedStep(input);
    const bool requiresFake = requiresFakeDoubleLoopRepresentation(
        input.marsClass(), input.marsStream());

    switch (input.timespanKind()) {
        case TimespanKind::Missing:
            if (nStat != 0) {
                resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                      "`stattype` is present while `timespan` is missing",
                                      input);
            }
            if (!input.options().allowMissingTimespanForInstantProduct) {
                resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                      "missing `timespan` instant compatibility option is disabled",
                                      input);
            }
            return ProductTimeSpecShapeKind::Instant;

        case TimespanKind::None:
            if (nStat == 0) {
                return ProductTimeSpecShapeKind::Instant;
            }
            if (nStat > 1) {
                resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                      "timespan='none' supports at most one stattype block",
                                      input);
            }
            if (!requiresFake) {
                resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                      "fakeDoubleLoop representation is not allowed for this class/stream",
                                      input);
            }
            return ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop;

        case TimespanKind::Duration:
            if (nStat == 0) {
                if (requiresFake) {
                    resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                          "standard single-loop representation is forbidden; fakeDoubleLoop is required",
                                          input);
                }
                return ProductTimeSpecShapeKind::StandardSingleLoop;
            }
            return ProductTimeSpecShapeKind::MultiLoop;

        case TimespanKind::FromStart:
            if (nStat != 0) {
                resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                      "from-start products forbid `stattype`",
                                      input);
            }
            if (step == 0 && !input.options().allowZeroLengthFsWindow) {
                resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                                      "zero-length from-start window is disabled",
                                      input);
            }
            return ProductTimeSpecShapeKind::FromStartSingleLoop;
    }

    resolver_detail::fail(ProductTimeSpecStage::ShapeClassification,
                          "unhandled timespan classification",
                          input);

    mars2gribUnreachable();
}

/**
 * @brief Construct one shape artifact from a valid shape classification.
 *
 * The function dispatches to one specialized shape constructor indexed by the
 * valid `ProductTimeSpecShapeKind` classification.
 *
 * @param input Normalized input snapshot.
 * @param shapeType Valid shape classification.
 * @param anchor Previously resolved anchor artifact.
 * @return Fully resolved `ProductTimeSpecShape` artifact.
 * @throws Mars2GribProductTimeSpecException on dispatch or construction
 *         failures.
 */
template <class Input_t>
ProductTimeSpecShape construct_ProductTimeSpecShape_or_throw(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType,
    const ProductTimeSpecAnchor& anchor) {
    using Builder = ProductTimeSpecShape (*)(const Input_t&, const ProductTimeSpecAnchor&);
    static const std::array<Builder,
                            static_cast<std::size_t>(ProductTimeSpecShapeKind::Count)>
        builders{
            &resolver_detail::constructInstantShape<Input_t>,
            &resolver_detail::constructStandardShape<Input_t>,
            &resolver_detail::constructMultiShape<Input_t>,
            &resolver_detail::constructFakeDoubleLoopShape<Input_t>,
            &resolver_detail::constructFromStartShape<Input_t>};

    const auto index = static_cast<std::size_t>(shapeType);
    if (index >= builders.size()) {
        resolver_detail::fail(ProductTimeSpecStage::ShapeConstruction,
                              "invalid shape dispatch index",
                              input);
    }
    try {
        return builders[index](input, anchor);
    } catch (const Mars2GribProductTimeSpecException&) {
        throw;
    } catch (const std::exception& e) {
        resolver_detail::fail(ProductTimeSpecStage::ShapeConstruction,
                              e.what(),
                              input);
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::product_time_spec

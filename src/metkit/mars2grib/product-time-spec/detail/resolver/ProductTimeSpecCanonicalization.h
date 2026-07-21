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
 * @file ProductTimeSpecCanonicalization.h
 * @brief Canonical ProductTimeSpec construction from resolver artifacts.
 */

#pragma once


#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::product_time_spec {

/**
 * @brief Build the final canonical ProductTimeSpec from construction artifacts.
 *
 * Canonicalization materializes the immutable ordered `ProductTimeWindow`
 * sequence and then embeds it together with the resolved anchor, support
 * interval, option snapshot, shape kind, and increment kind into the final
 * ProductTimeSpec object.
 *
 * Per-window increment materialization follows the ProductTimeSpec model:
 *
 * - the innermost real window uses the resolved increment artifact;
 * - outer multi-loop windows use the immediately inner window range as their
 *   increment;
 * - instant products receive one zero-length placeholder window;
 * - AIFS pure-missing increment products receive the missing-increment
 *   sentinel.
 *
 * Construction also checks local canonical-window invariants, including:
 *
 * - real windows having non-missing processing types;
 * - positive materialized increments for non-sentinel cases;
 * - positive real window lengths except the authorized zero-length from-start
 *   branch;
 * - increment fitting inside its corresponding window.
 *
 * @param input Normalized input snapshot.
 * @param classification Valid classification triple.
 * @param anchor Resolved anchor artifact.
 * @param shape Resolved shape artifact.
 * @param increment Resolved increment artifact.
 * @return Final immutable ProductTimeSpec.
 * @throws Mars2GribProductTimeSpecException on canonical-window or final-object
 *         construction failures.
 */
template <class Input_t>
ProductTimeSpec canonicalize_ProductTimeSpec_or_throw(
    const Input_t& input,
    const ProductTimeSpecClassification& classification,
    ProductTimeSpecAnchor anchor,
    const ProductTimeSpecShape& shape,
    const ProductTimeSpecIncrement& increment) {
    try {
        ProductTimeWindows windows;
        const auto innerType = input.innerMostTypeOfStatisticalProcessing();

        switch (classification.shapeType) {
            case ProductTimeSpecShapeKind::Instant:
                windows.append(ProductTimeWindow{
                    tables::TypeOfStatisticalProcessing::Missing,
                    TypeOfTimeIncrement::Missing,
                    ProductTimeDuration{tables::TimeUnit::Second, 0},
                    ProductTimeDuration{tables::TimeUnit::Second, 0}});
                break;

            case ProductTimeSpecShapeKind::StandardSingleLoop:
            case ProductTimeSpecShapeKind::FromStartSingleLoop:
                windows.append(ProductTimeWindow{innerType,
                                                 increment.typeOfTimeIncrement,
                                                 *shape.innerTimeRange,
                                                 increment.timeIncrement});
                break;

            case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
                windows.append(ProductTimeWindow{
                    shape.stattypeBlocks[0].typeOfStatisticalProcessing,
                    increment.typeOfTimeIncrement,
                    shape.stattypeBlocks[0].timeRange,
                    increment.timeIncrement});
                break;

            case ProductTimeSpecShapeKind::MultiLoop:
                for (std::size_t i = 0; i < shape.stattypeBlocks.size(); ++i) {
                    const ProductTimeDuration innerRange =
                        (i + 1 < shape.stattypeBlocks.size())
                            ? shape.stattypeBlocks[i + 1].timeRange
                            : *shape.innerTimeRange;
                    windows.append(ProductTimeWindow{
                        shape.stattypeBlocks[i].typeOfStatisticalProcessing,
                        increment.typeOfTimeIncrement,
                        shape.stattypeBlocks[i].timeRange,
                        innerRange});
                }
                windows.append(ProductTimeWindow{innerType,
                                                 increment.typeOfTimeIncrement,
                                                 *shape.innerTimeRange,
                                                 increment.timeIncrement});
                break;

            case ProductTimeSpecShapeKind::Count:
                resolver_detail::fail(ProductTimeSpecStage::CanonicalWindowConstruction,
                                      "invalid canonicalization shape sentinel",
                                      input,
                                      classification);
        }

        for (std::size_t i = 0; i < windows.size(); ++i) {
            const auto& window = windows[i];
            const bool zeroFromStart =
                classification.shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop &&
                resolver_detail::resolvedStep(input) == 0;

            if (classification.shapeType != ProductTimeSpecShapeKind::Instant &&
                resolver_detail::isMissing(window.typeOfStatisticalProcessing)) {
                resolver_detail::fail(ProductTimeSpecStage::CanonicalWindowConstruction,
                                      "real statistical window has Missing processing type",
                                      input,
                                      classification);
            }
            if (classification.shapeType != ProductTimeSpecShapeKind::Instant &&
                window.timeRange.length <= 0 && !zeroFromStart) {
                resolver_detail::fail(ProductTimeSpecStage::CanonicalWindowConstruction,
                                      "real statistical window has non-positive range",
                                      input,
                                      classification);
            }

            const bool missingIncrementSentinel =
                classification.incrementType == TimeIncrementKind::AifsPureMissingIncrement;
            if (classification.shapeType != ProductTimeSpecShapeKind::Instant &&
                !missingIncrementSentinel &&
                (resolver_detail::isMissing(window.typeOfTimeIncrement) ||
                 window.timeIncrement.length <= 0)) {
                resolver_detail::fail(ProductTimeSpecStage::CanonicalWindowConstruction,
                                      "real statistical window has no materialized positive increment",
                                      input,
                                      classification);
            }

            if (classification.shapeType != ProductTimeSpecShapeKind::Instant &&
                !zeroFromStart) {
                const eckit::DateTime realizedStart =
                    subtractDuration(shape.windowEndDateTime, window.timeRange);
                if (!durationFitsAt(realizedStart, window.timeIncrement, window.timeRange)) {
                    resolver_detail::fail(ProductTimeSpecStage::CanonicalWindowConstruction,
                                          "time increment exceeds its statistical window",
                                          input,
                                          classification);
                }
            }
        }

        return ProductTimeSpec(std::move(anchor),
                               shape.windowStartDateTime,
                               shape.windowEndDateTime,
                               std::move(windows),
                               input.options(),
                               classification.shapeType,
                               classification.incrementType);
    } catch (const Mars2GribProductTimeSpecException&) {
        throw;
    } catch (const std::exception& e) {
        resolver_detail::fail(ProductTimeSpecStage::ProductTimeSpecConstruction,
                              e.what(),
                              input,
                              classification);
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::product_time_spec

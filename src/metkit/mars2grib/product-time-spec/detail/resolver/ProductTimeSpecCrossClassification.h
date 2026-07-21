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
 * @file ProductTimeSpecCrossClassification.h
 * @brief Cross-axis semantic consistency checks for ProductTimeSpec classification.
 *
 * This header validates relationships that must hold across independently valid
 * anchor, shape, and increment classifications before any construction artifact
 * is built.
 */

#pragma once


#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::product_time_spec {

/**
 * @brief Validate relationships between independently valid classifications.
 *
 * Anchor, shape, and increment classification are intentionally resolved as
 * separate axes. This function verifies the semantic relationships that must
 * hold across those axes before any construction artifact is built.
 *
 * The checks include:
 *
 * - instant products requiring missing innermost processing and `NoIncrement`;
 * - statistical products requiring non-missing innermost processing;
 * - from-start products requiring innermost Accumulation processing;
 * - fake-double-loop processing agreement between `stattype` and caller input;
 * - `AifsPureMissingIncrement` validity for `class="ml"` single-real-window
 *   products only;
 * - `DefaultedIncrement` exclusion for `ml` and from-start products.
 *
 * @param input Normalized input snapshot.
 * @param classification Fully resolved valid classification triple.
 * @throws Mars2GribProductTimeSpecException if the combination is semantically
 *         inconsistent.
 */
template <class Input_t>
void check_CrossClassificationConsistency_or_throw(
    const Input_t& input,
    const ProductTimeSpecClassification& classification) {
    const auto inner = input.innerMostTypeOfStatisticalProcessing();

    if (classification.shapeType == ProductTimeSpecShapeKind::Instant) {
        if (inner != tables::TypeOfStatisticalProcessing::Missing) {
            resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                                  "instant product requires Missing innermost statistical processing",
                                  input,
                                  classification);
        }
        if (classification.incrementType != TimeIncrementKind::NoIncrement) {
            resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                                  "instant product requires NoIncrement",
                                  input,
                                  classification);
        }
        return;
    }

    if (inner == tables::TypeOfStatisticalProcessing::Missing) {
        resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                              "statistical product requires non-Missing innermost processing",
                              input,
                              classification);
    }

    if (classification.shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop &&
        inner != tables::TypeOfStatisticalProcessing::Accumulation) {
        resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                              "from-start product requires Accumulation processing",
                              input,
                              classification);
    }

    if (classification.shapeType ==
        ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop) {
        if (input.stattypeBlocks().size() != 1 ||
            input.stattypeBlocks()[0].typeOfStatisticalProcessing != inner) {
            resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                                  "fakeDoubleLoop stattype processing disagrees with caller-supplied innermost processing",
                                  input,
                                  classification);
        }
    }

    const std::size_t realCount = realStatisticalWindowCount(
        classification.shapeType, input.stattypeBlocks().size());
    if (classification.incrementType == TimeIncrementKind::AifsPureMissingIncrement &&
        (input.marsClass() != "ml" || realCount != 1)) {
        resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                              "AifsPureMissingIncrement requires class='ml' and exactly one real window",
                              input,
                              classification);
    }
    if (classification.incrementType == TimeIncrementKind::DefaultedIncrement &&
        (input.marsClass() == "ml" ||
         classification.shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop)) {
        resolver_detail::fail(ProductTimeSpecStage::ClassificationConsistencyCheck,
                              "DefaultedIncrement is not valid for ml or from-start products",
                              input,
                              classification);
    }
}

}  // namespace metkit::mars2grib::product_time_spec

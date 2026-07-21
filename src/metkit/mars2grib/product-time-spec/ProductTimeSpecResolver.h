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
 * @file ProductTimeSpecResolver.h
 * @brief ProductTimeSpec frontend orchestration and canonicalization layer.
 *
 * This header is the public entry point of the ProductTimeSpec frontend
 * resolver. After the split into dedicated anchor, shape, and increment detail
 * headers, it remains responsible for the parts of the resolver that span
 * multiple stages:
 *
 * - inclusion and orchestration of the specialized resolver components;
 * - the top-level classifier-first resolution pipeline.
 *
 * The specialized stage headers are:
 *
 * - `detail/ProductTimeSpecAnchorResolver.h`;
 * - `detail/ProductTimeSpecShapeResolver.h`;
 * - `detail/ProductTimeSpecIncrementResolver.h`.
 * - `detail/ProductTimeSpecCrossClassification.h`;
 * - `detail/ProductTimeSpecCanonicalization.h`;
 * - `detail/ProductTimeSpecFinalConsistency.h`.
 *
 * Ownership boundary:
 *
 * - this header does own semantic orchestration after a normalized
 *   `ProductTimeSpecInput` snapshot exists;
 * - this header does not own dictionary access, lexical parsing, type
 *   normalization, or raw option extraction; those belong to
 *   `ProductTimeSpecInput.h`.
 *
 * Error model:
 *
 * - once a complete input snapshot exists, failures are reported as
 *   `Mars2GribProductTimeSpecException`;
 * - every failure is tagged with the corresponding `ProductTimeSpecStage` and
 *   carries the serialized input snapshot, plus any available classification,
 *   artifact, or final-object context.
 *
 * The implementation follows the resolver pipeline and canonicalization rules
 * documented in `productTimeSpecV3_final.md`, especially Sections 3.2, 3.3,
 * 3.4, 5.4, 5.5, 5.6, and the final-invariant sections.
 */

#pragma once

#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecResolverCommon.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecAnchorResolver.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecShapeResolver.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecIncrementResolver.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecCrossClassification.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecCanonicalization.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecFinalConsistency.h"


namespace metkit::mars2grib::product_time_spec {

/**
 * @brief Resolve one normalized ProductTimeSpec input into the final IR.
 *
 * The pipeline is intentionally classifier-first:
 *
 * 1. classify anchor;
 * 2. classify shape;
 * 3. classify increment;
 * 4. validate cross-classification consistency;
 * 5. construct anchor;
 * 6. construct shape;
 * 7. construct increment;
 * 8. canonicalize the final ProductTimeSpec;
 * 9. validate final whole-object invariants.
 *
 * This function is the single entry point from the deduction layer into the
 * ProductTimeSpec frontend resolver.
 *
 * @param input Normalized input snapshot.
 * @return Final canonical ProductTimeSpec.
 * @throws Mars2GribProductTimeSpecException if any resolver stage fails.
 */
template <class Input_t>
ProductTimeSpec resolve_ProductTimeSpecInput_or_throw(const Input_t& input) {
    ProductTimeSpecClassification classification;
    classification.anchorType = classify_TimeAnchor_or_throw(input);
    classification.shapeType = classify_ProductTimeSpecShape_or_throw(input);
    classification.incrementType =
        classify_TimeIncrement_or_throw(input, classification.shapeType);

    check_CrossClassificationConsistency_or_throw(input, classification);

    auto anchor = construct_ProductTimeSpecAnchor_or_throw(
        input, classification.anchorType);
    auto shape = construct_ProductTimeSpecShape_or_throw(
        input, classification.shapeType, anchor);
    auto increment = construct_ProductTimeSpecIncrement_or_throw(
        input, shape, classification.incrementType);

    auto result = canonicalize_ProductTimeSpec_or_throw(
        input,
        classification,
        std::move(anchor),
        shape,
        increment);

    check_FinalConsistency_or_throw(input, classification, result);
    return result;
}

}  // namespace metkit::mars2grib::product_time_spec

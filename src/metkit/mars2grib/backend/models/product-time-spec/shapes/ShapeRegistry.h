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
/// @file ShapeRegistry.h
/// @brief Register, classify, and dispatch ProductTimeSpec shape cases.
///
/// Shape classification is exhaustive and non-prioritized. Every matcher is
/// evaluated, all Boolean results are retained for diagnostics, and exactly one
/// matcher must succeed. This turns accidental matcher overlap into a visible hard
/// error rather than silently selecting the first row.
///
/// The active callback-selection matrix is summarized case-by-case below. Each
/// row lists the matcher facts that must simultaneously hold for that callback
/// to be selected.
///
/// | Shape case | Regime | Domain kind | Synoptic | `timespan.kind` | `stattype` | `timeIncrement` expectation |
/// Fake-double-loop flag | Fake-second-loop flag |
/// |------------|--------|-------------|----------|------------------|------------|-----------------------------|----------------------|----------------------|
/// | `Instant` | any | any | any | `None`, or `Missing` when allowed | empty | redundant values validated later | n/a |
/// n/a | | `IFSStandardSingleLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | empty | explicit, missing, or
/// defaulted | `false` | `false` | | `IFSFakeDoubleLoopSingleLoop` | `IFS` | `ForecastDomain` | `false` | `None`, or
/// `Missing` when allowed | exactly one block | explicit, missing, or defaulted | `true` | `false` | |
/// `IFSFromStartSingleLoopAtZero` | `IFS` | `ForecastDomain` | `false` | `FromStart` | empty |
/// explicit, missing, or defaulted | n/a | n/a | | `IFSFromStartSingleLoopPositive` | `IFS` | `ForecastDomain` |
/// `false` | `FromStart` | empty | explicit, missing, or defaulted | n/a | n/a | | `IFSSynopticSingleLoop` |
/// `IFS` | `SynopticAnalysisDomain` | `true` | synoptic-supported source | empty | intrinsic or redundant 24h value |
/// n/a | n/a | | `AIFSStandardSingleLoop` | `AIFS` | `ForecastDomain` | `false` | `Duration` | empty | must be missing
/// | `false` | `false` | | `AIFSFakeDoubleLoopSingleLoop` | `AIFS` | `ForecastDomain` | `false` | `None`, or `Missing`
/// when allowed | exactly one block | must be missing | `true` | n/a | | `AIFSFromStartSingleLoopAtZero` | `AIFS` |
/// `ForecastDomain` | `false` | `FromStart` | empty | must be missing | n/a | n/a | | `AIFSFromStartSingleLoopPositive`
/// | `AIFS` | `ForecastDomain` | `false` | `FromStart` | empty | must be missing | n/a | n/a | | `SeasonalSingleLoop` |
/// any | `SeasonalForecastDomain` | `false` | `None`, or `Missing` when allowed | empty | explicit, missing, or
/// defaulted | n/a | n/a | | `SeasonalMultiloop` | any | `SeasonalForecastDomain` | `false` | `Duration` | one or more
/// blocks | explicit, missing, or defaulted | n/a | n/a | | `IFSStandardMultiLoop` | `IFS` | `ForecastDomain` | `false`
/// | `Duration` | one or more blocks | explicit, missing, or defaulted | n/a | n/a | | `IFSFakeSingleLoopDoubleLoop` |
/// `IFS` | `ForecastDomain` | `false` | `Duration` | empty | explicit, missing, or defaulted | `false` | `true` |
///
/// @ingroup mars2grib_product_time_spec_shapes
///
#pragma once

#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/AIFSFakeDoubleLoopSingleLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/AIFSFromStartSingleLoopAtZero.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/AIFSFromStartSingleLoopPositive.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/AIFSStandardSingleLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSFakeDoubleLoopSingleLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSFakeSingleLoopDoubleLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSFromStartSingleLoopAtZero.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSFromStartSingleLoopPositive.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSStandardMultiLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSStandardSingleLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/IFSSynopticSingleLoop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/Instant.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/SeasonalMultiloop.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/SeasonalSingleLoop.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape {

namespace detail {

/// @brief Function-pointer type shared by all shape matchers.
using ShapeMatcher = bool (*)(const ProductTimeSpecInput&);

/// @brief Function-pointer type shared by all stage-1 shape builders.
using ShapeStage1Builder = ProductTimeSpecShapeStage1 (*)(const ProductTimeSpecInput&,
                                                          const ProductTimeSpecClassification&);

/// @brief Function-pointer type shared by all final shape builders.
using ShapeFinalBuilder = ProductTimeSpecShape (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&,
                                                   const anchor::ProductTimeSpecAnchor&,
                                                   const ProductTimeSpecShapeStage1&,
                                                   const domain::ProductTimeSpecDomain&);

///
/// @brief Immutable registry row for one shape case.
///
/// Keeping the classification value, diagnostic name, matcher, and builders in
/// one object prevents registry arrays from drifting out of alignment.
///
struct ShapeCase {
    ProductTimeSpecShapeKind classification;
    std::string_view name;
    ShapeMatcher matcher;
    ShapeStage1Builder stage1Builder;
    ShapeFinalBuilder finalBuilder;
};

/// @brief Immutable shape registry ordered exactly like `ProductTimeSpecShapeKind`.
inline constexpr std::array<ShapeCase, static_cast<std::size_t>(ProductTimeSpecShapeKind::Count)> shapeCases{{
    {ProductTimeSpecShapeKind::Instant, "Instant", &match_Instant_Shape, &build_Instant_ShapeStage1,
     &build_Instant_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSStandardSingleLoop, "IFSStandardSingleLoop", &match_IFSStandardSingleLoop_Shape,
     &build_IFSStandardSingleLoop_ShapeStage1, &build_IFSStandardSingleLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSFakeDoubleLoopSingleLoop, "IFSFakeDoubleLoopSingleLoop",
     &match_IFSFakeDoubleLoopSingleLoop_Shape, &build_IFSFakeDoubleLoopSingleLoop_ShapeStage1,
     &build_IFSFakeDoubleLoopSingleLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSFromStartSingleLoopAtZero, "IFSFromStartSingleLoopAtZero",
     &match_IFSFromStartSingleLoopAtZero_Shape, &build_IFSFromStartSingleLoopAtZero_ShapeStage1,
     &build_IFSFromStartSingleLoopAtZero_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSFromStartSingleLoopPositive, "IFSFromStartSingleLoopPositive",
     &match_IFSFromStartSingleLoopPositive_Shape, &build_IFSFromStartSingleLoopPositive_ShapeStage1,
     &build_IFSFromStartSingleLoopPositive_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSSynopticSingleLoop, "IFSSynopticSingleLoop", &match_IFSSynopticSingleLoop_Shape,
     &build_IFSSynopticSingleLoop_ShapeStage1, &build_IFSSynopticSingleLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::AIFSStandardSingleLoop, "AIFSStandardSingleLoop", &match_AIFSStandardSingleLoop_Shape,
     &build_AIFSStandardSingleLoop_ShapeStage1, &build_AIFSStandardSingleLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop, "AIFSFakeDoubleLoopSingleLoop",
     &match_AIFSFakeDoubleLoopSingleLoop_Shape, &build_AIFSFakeDoubleLoopSingleLoop_ShapeStage1,
     &build_AIFSFakeDoubleLoopSingleLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::AIFSFromStartSingleLoopAtZero, "AIFSFromStartSingleLoopAtZero",
     &match_AIFSFromStartSingleLoopAtZero_Shape, &build_AIFSFromStartSingleLoopAtZero_ShapeStage1,
     &build_AIFSFromStartSingleLoopAtZero_ShapeFinal},
    {ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive, "AIFSFromStartSingleLoopPositive",
     &match_AIFSFromStartSingleLoopPositive_Shape, &build_AIFSFromStartSingleLoopPositive_ShapeStage1,
     &build_AIFSFromStartSingleLoopPositive_ShapeFinal},
    {ProductTimeSpecShapeKind::SeasonalSingleLoop, "SeasonalSingleLoop", &match_SeasonalSingleLoop_Shape,
     &build_SeasonalSingleLoop_ShapeStage1, &build_SeasonalSingleLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::SeasonalMultiloop, "SeasonalMultiloop", &match_SeasonalMultiloop_Shape,
     &build_SeasonalMultiloop_ShapeStage1, &build_SeasonalMultiloop_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSStandardMultiLoop, "IFSStandardMultiLoop", &match_IFSStandardMultiLoop_Shape,
     &build_IFSStandardMultiLoop_ShapeStage1, &build_IFSStandardMultiLoop_ShapeFinal},
    {ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop, "IFSFakeSingleLoopDoubleLoop",
     &match_IFSFakeSingleLoopDoubleLoop_Shape, &build_IFSFakeSingleLoopDoubleLoop_ShapeStage1,
     &build_IFSFakeSingleLoopDoubleLoop_ShapeFinal},
}};

static_assert(static_cast<std::size_t>(shapeCases[0].classification) == 0);
static_assert(static_cast<std::size_t>(shapeCases[1].classification) == 1);
static_assert(static_cast<std::size_t>(shapeCases[2].classification) == 2);
static_assert(static_cast<std::size_t>(shapeCases[3].classification) == 3);
static_assert(static_cast<std::size_t>(shapeCases[4].classification) == 4);
static_assert(static_cast<std::size_t>(shapeCases[5].classification) == 5);
static_assert(static_cast<std::size_t>(shapeCases[6].classification) == 6);
static_assert(static_cast<std::size_t>(shapeCases[7].classification) == 7);
static_assert(static_cast<std::size_t>(shapeCases[8].classification) == 8);
static_assert(static_cast<std::size_t>(shapeCases[9].classification) == 9);
static_assert(static_cast<std::size_t>(shapeCases[10].classification) == 10);
static_assert(static_cast<std::size_t>(shapeCases[11].classification) == 11);
static_assert(static_cast<std::size_t>(shapeCases[12].classification) == 12);
static_assert(static_cast<std::size_t>(shapeCases[13].classification) == 13);

}  // namespace detail

///
/// @brief Classify the normalized input against every registered shape case.
///
/// @section Shape classification contract
/// - Reads: normalized source facts and embedded options.
/// - Evaluates: every matcher in `shapeCases`.
/// - Success: exactly one matcher returns `true`.
/// - Failure: zero or multiple matchers return `true`.
/// - Side effects: none.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input.
///
/// @return
/// Unique matching `ProductTimeSpecShapeKind` value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If matcher evaluation fails or classification is not unique.
///
inline ProductTimeSpecShapeKind classify_Shape_or_throw(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        std::array<bool, detail::shapeCases.size()> matches{};
        std::size_t numberOfMatches = 0;
        std::size_t matchedIndex    = 0;

        for (std::size_t i = 0; i < detail::shapeCases.size(); ++i) {
            matches[i] = detail::shapeCases[i].matcher(input);

            if (matches[i]) {
                ++numberOfMatches;
                matchedIndex = i;
            }
        }

        if (numberOfMatches != 1) {
            throw Mars2GribModelException(
                [&]() {
                    std::ostringstream oss;
                    oss << "Shape classification failed: expected exactly one match, but found " << numberOfMatches
                        << " matches. Match results: [";
                    for (std::size_t i = 0; i < matches.size(); ++i) {
                        oss << detail::shapeCases[i].name << "=" << (matches[i] ? "true" : "false");
                        if (i < matches.size() - 1) {
                            oss << ", ";
                        }
                    }
                    oss << "]";
                    return oss.str();
                }(),
                input.to_json(), Here());
        }

        return detail::shapeCases[matchedIndex].classification;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to classify the ProductTimeSpec shape", input.to_json(), Here()));
    }
}

///
/// @brief Dispatch the stage-1 builder associated with a validated shape classification.
///
/// @param[in] classification
/// Unique shape classification returned by `classify_Shape_or_throw`.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input, including embedded options.
///
/// @param[in] fullClassification
/// Full resolved ProductTimeSpec classification bundle.
///
/// @return
/// Stage-1 ProductTimeSpec windows ordered outermost to innermost.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If the classification is invalid or the selected stage-1 builder fails.
///
inline ProductTimeSpecShapeStage1 build_ShapeStage1_or_throw(ProductTimeSpecShapeKind classification,
                                                             const ProductTimeSpecInput& input,
                                                             const ProductTimeSpecClassification& fullClassification) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const bool classificationIsValid = index < detail::shapeCases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecShapeKind value", input.to_json(), Here());
        }

        return detail::shapeCases[index].stage1Builder(input, fullClassification);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec stage-1 shape", input.to_json(), Here()));
    }
}

///
/// @brief Dispatch the final builder associated with a validated shape classification.
///
/// @param[in] classification
/// Unique shape classification returned by `classify_Shape_or_throw`.
/// @param[in] input Fully normalized ProductTimeSpec input, including embedded options.
/// @param[in] classificationBundle Full resolved ProductTimeSpec classification bundle.
/// @param[in] anchor Resolved ProductTimeSpec anchor.
/// @param[in] shapeStage1 Stage-1 ProductTimeSpec shape artifact.
/// @param[in] domain Resolved ProductTimeSpec domain.
/// @return Final canonical ProductTimeSpec shape.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException If the classification is invalid or the
///         selected final builder fails.
///
inline ProductTimeSpecShape build_ShapeFinal_or_throw(ProductTimeSpecShapeKind classification,
                                                      const ProductTimeSpecInput& input,
                                                      const ProductTimeSpecClassification& classificationBundle,
                                                      const anchor::ProductTimeSpecAnchor& anchor,
                                                      const ProductTimeSpecShapeStage1& shapeStage1,
                                                      const domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const bool classificationIsValid = index < detail::shapeCases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecShapeKind value", input.to_json(), Here());
        }

        return detail::shapeCases[index].finalBuilder(input, classificationBundle, anchor, shapeStage1, domain);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the final ProductTimeSpec shape", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape

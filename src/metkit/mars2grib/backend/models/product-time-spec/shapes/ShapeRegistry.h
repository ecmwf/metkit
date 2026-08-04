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
/// | `InstantTimespanMissing` | any | any | any | `Missing` | empty | redundant values validated later | n/a | n/a |
/// | `InstantTimespanNone` | any | any | any | `None` | empty | redundant values validated later | n/a | n/a |
/// | `IFSStandardSingleLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | empty | explicit,
/// missing, or defaulted | `false` | `false` | | `IFSFakeDoubleLoopSingleLoop` | `IFS` | `ForecastDomain` |
/// `false` | `None` | exactly one block | explicit, missing, or defaulted | `true` | `false` | |
/// `IFSFromStartSingleLoopAtZero` | `IFS` | `ForecastDomain` | `false` | `FromStart` | empty |
/// explicit, missing, or defaulted | n/a | n/a | | `IFSFromStartSingleLoopPositive` | `IFS` | `ForecastDomain` |
/// `false` | `FromStart` | empty | explicit, missing, or defaulted | n/a | n/a | | `IFSSynopticSingleLoop` |
/// `IFS` | `SynopticAnalysisDomain` | `true` | synoptic-supported source | empty | intrinsic or redundant 24h value |
/// n/a | n/a | | `AIFSStandardSingleLoop` | `AIFS` | `ForecastDomain` | `false` | `Duration` | empty | must be missing
/// | `false` | `false` | | `AIFSFakeDoubleLoopSingleLoop` | `AIFS` | `ForecastDomain` | `false` | `None` | exactly one
/// block | must be missing | `true` | n/a | | `AIFSFromStartSingleLoopAtZero` | `AIFS` | `ForecastDomain` | `false` |
/// `FromStart` | empty | must be missing | n/a | n/a | | `AIFSFromStartSingleLoopPositive` | `AIFS` | `ForecastDomain`
/// | `false` | `FromStart` | empty | must be missing | n/a | n/a | | `SeasonalSingleLoop` | any |
/// `SeasonalForecastDomain` | `false` | `Duration` | empty | explicit, missing, or defaulted | n/a | n/a | |
/// `IFSStandardMultiLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | one or more blocks | explicit, missing,
/// or defaulted | n/a | n/a | | `IFSFakeSingleLoopDoubleLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | empty
/// | explicit, missing, or defaulted | `false` | `true` |
///
/// @ingroup mars2grib_product_time_spec_shapes
///
#pragma once

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
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/InstantTimespanMissing.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/InstantTimespanNone.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/impl/SeasonalSingleLoop.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape {

namespace detail {

/// @brief Function-pointer type shared by all shape matchers.
using ShapeMatcher =
    bool (*)(const ProductTimeSpecInput&,
             const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind&);

/// @brief Function-pointer type shared by all shape builders.
using ShapeBuilder = std::vector<ProductTimeSpecWindow> (*)(
    const ProductTimeSpecInput&,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain&);


///
/// @brief Immutable registry row for one shape case.
///
/// Keeping the classification value, diagnostic name, matcher, and builder in one
/// object prevents registry arrays from drifting out of alignment.
///
struct ShapeCase {
    ProductTimeSpecShapeKind classification;
    std::string_view name;
    ShapeMatcher matcher;
    ShapeBuilder builder;
};

/// @brief Immutable shape registry ordered exactly like `ProductTimeSpecShapeKind`.
inline constexpr std::array<ShapeCase, static_cast<std::size_t>(ProductTimeSpecShapeKind::Count)> shapeCases{{
    {ProductTimeSpecShapeKind::InstantTimespanMissing, "InstantTimespanMissing", &match_InstantTimespanMissing_Shape,
     &build_InstantTimespanMissing_Shape},
    {ProductTimeSpecShapeKind::InstantTimespanNone, "InstantTimespanNone", &match_InstantTimespanNone_Shape,
     &build_InstantTimespanNone_Shape},
    {ProductTimeSpecShapeKind::IFSStandardSingleLoop, "IFSStandardSingleLoop", &match_IFSStandardSingleLoop_Shape,
     &build_IFSStandardSingleLoop_Shape},
    {ProductTimeSpecShapeKind::IFSFakeDoubleLoopSingleLoop, "IFSFakeDoubleLoopSingleLoop",
     &match_IFSFakeDoubleLoopSingleLoop_Shape, &build_IFSFakeDoubleLoopSingleLoop_Shape},
    {ProductTimeSpecShapeKind::IFSFromStartSingleLoopAtZero, "IFSFromStartSingleLoopAtZero",
     &match_IFSFromStartSingleLoopAtZero_Shape, &build_IFSFromStartSingleLoopAtZero_Shape},
    {ProductTimeSpecShapeKind::IFSFromStartSingleLoopPositive, "IFSFromStartSingleLoopPositive",
     &match_IFSFromStartSingleLoopPositive_Shape, &build_IFSFromStartSingleLoopPositive_Shape},
    {ProductTimeSpecShapeKind::IFSSynopticSingleLoop, "IFSSynopticSingleLoop", &match_IFSSynopticSingleLoop_Shape,
     &build_IFSSynopticSingleLoop_Shape},
    {ProductTimeSpecShapeKind::AIFSStandardSingleLoop, "AIFSStandardSingleLoop", &match_AIFSStandardSingleLoop_Shape,
     &build_AIFSStandardSingleLoop_Shape},
    {ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop, "AIFSFakeDoubleLoopSingleLoop",
     &match_AIFSFakeDoubleLoopSingleLoop_Shape, &build_AIFSFakeDoubleLoopSingleLoop_Shape},
    {ProductTimeSpecShapeKind::AIFSFromStartSingleLoopAtZero, "AIFSFromStartSingleLoopAtZero",
     &match_AIFSFromStartSingleLoopAtZero_Shape, &build_AIFSFromStartSingleLoopAtZero_Shape},
    {ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive, "AIFSFromStartSingleLoopPositive",
     &match_AIFSFromStartSingleLoopPositive_Shape, &build_AIFSFromStartSingleLoopPositive_Shape},
    {ProductTimeSpecShapeKind::SeasonalSingleLoop, "SeasonalSingleLoop", &match_SeasonalSingleLoop_Shape,
     &build_SeasonalSingleLoop_Shape},
    {ProductTimeSpecShapeKind::IFSStandardMultiLoop, "IFSStandardMultiLoop", &match_IFSStandardMultiLoop_Shape,
     &build_IFSStandardMultiLoop_Shape},
    {ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop, "IFSFakeSingleLoopDoubleLoop",
     &match_IFSFakeSingleLoopDoubleLoop_Shape, &build_IFSFakeSingleLoopDoubleLoop_Shape},
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
/// - Reads: normalized source facts, embedded options, and domain classification.
/// - Evaluates: every matcher in `shapeCases`.
/// - Success: exactly one matcher returns `true`.
/// - Failure: zero or multiple matchers return `true`.
/// - Side effects: none.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input.
///
/// @param[in] domainClassification
/// Unique domain classification used by cross-axis shape constraints.
///
/// @return
/// Unique matching `ProductTimeSpecShapeKind` value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If matcher evaluation fails or classification is not unique.
///
inline ProductTimeSpecShapeKind classify_Shape_or_throw(
    const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomainKind& domainKind) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        std::array<bool, detail::shapeCases.size()> matches{};
        std::size_t numberOfMatches = 0;
        std::size_t matchedIndex    = 0;

        for (std::size_t i = 0; i < detail::shapeCases.size(); ++i) {
            matches[i] = detail::shapeCases[i].matcher(input, domainKind);

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
/// @brief Dispatch the leaf builder associated with a validated shape classification.
///
/// @param[in] classification
/// Unique shape classification returned by `classify_Shape_or_throw`.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input, including embedded options.
///
/// @param[in] domain
/// Absolute ProductTimeSpec domain constructed before window construction.
///
/// @return
/// Canonical ProductTimeSpec windows ordered outermost to innermost.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If the classification is invalid or the selected leaf builder fails.
///
inline std::vector<ProductTimeSpecWindow> build_Shape_or_throw(
    ProductTimeSpecShapeKind classification, const ProductTimeSpecInput& input,
    const metkit::mars2grib::backend::models::product_time_spec::domain::ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const bool classificationIsValid = index < detail::shapeCases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecShapeKind value", input.to_json(), Here());
        }

        return detail::shapeCases[index].builder(input, domain);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec shape", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape

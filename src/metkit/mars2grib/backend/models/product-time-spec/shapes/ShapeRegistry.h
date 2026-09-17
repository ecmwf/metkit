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
/// @brief Register, classify, build, and check ProductTimeSpec shape cases.
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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
template <class Cntx_t>
using ShapeMatcher = bool (*)(const ProductTimeSpecInput&, Cntx_t&);

/// @brief Function-pointer type shared by all stage-1 shape builders.
template <class Cntx_t>
using ShapeOuterTimeRangeBuilder = ProductTimeSpecOuterTimeRange (*)(const ProductTimeSpecInput&,
                                                                     const ProductTimeSpecClassification&, Cntx_t&);

/// @brief Function-pointer type shared by all final shape builders.
template <class Cntx_t>
using ShapeWindowsBuilder = ProductTimeSpecShape (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&,
                                                     const anchor::ProductTimeSpecAnchor&,
                                                     const ProductTimeSpecOuterTimeRange&,
                                                     const domain::ProductTimeSpecDomain&, Cntx_t&);

/// @brief Function-pointer type shared by all shape check callbacks.
template <class Cntx_t>
using ShapeChecker = bool (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&,
                              const anchor::ProductTimeSpecAnchor&, const ProductTimeSpecOuterTimeRange&,
                              const domain::ProductTimeSpecDomain&, const ProductTimeSpecShape&, Cntx_t&);

///
/// @brief Immutable registry row for one shape case.
///
/// Keeping the classification value, diagnostic name, matcher, and builders in
/// one object prevents registry arrays from drifting out of alignment.
///
template <class Cntx_t>
struct ShapeCase {
    ProductTimeSpecShapeKind classification;
    std::string_view name;
    ShapeMatcher<Cntx_t> matcher;
    ShapeOuterTimeRangeBuilder<Cntx_t> outerTimeRangeBuilder;
    ShapeWindowsBuilder<Cntx_t> windowsBuilder;
    ShapeChecker<Cntx_t> checker;
};

/// @brief Immutable shape registry ordered exactly like `ProductTimeSpecShapeKind`.
template <class Cntx_t>
inline constexpr std::array<ShapeCase<Cntx_t>, static_cast<std::size_t>(ProductTimeSpecShapeKind::Count)> shapeCases{{
    {ProductTimeSpecShapeKind::Instant, "Instant", &match_Instant_Shape<Cntx_t>, &build_Instant_ShapeOuterTimeRange<Cntx_t>,
     &build_Instant_ShapeWindows<Cntx_t>, &check_Instant_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSStandardSingleLoop, "IFSStandardSingleLoop", &match_IFSStandardSingleLoop_Shape<Cntx_t>,
     &build_IFSStandardSingleLoop_ShapeOuterTimeRange<Cntx_t>, &build_IFSStandardSingleLoop_ShapeWindows<Cntx_t>,
     &check_IFSStandardSingleLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSFakeDoubleLoopSingleLoop, "IFSFakeDoubleLoopSingleLoop",
     &match_IFSFakeDoubleLoopSingleLoop_Shape<Cntx_t>, &build_IFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange<Cntx_t>,
     &build_IFSFakeDoubleLoopSingleLoop_ShapeWindows<Cntx_t>, &check_IFSFakeDoubleLoopSingleLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSFromStartSingleLoopAtZero, "IFSFromStartSingleLoopAtZero",
     &match_IFSFromStartSingleLoopAtZero_Shape<Cntx_t>, &build_IFSFromStartSingleLoopAtZero_ShapeOuterTimeRange<Cntx_t>,
     &build_IFSFromStartSingleLoopAtZero_ShapeWindows<Cntx_t>, &check_IFSFromStartSingleLoopAtZero_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSFromStartSingleLoopPositive, "IFSFromStartSingleLoopPositive",
     &match_IFSFromStartSingleLoopPositive_Shape<Cntx_t>, &build_IFSFromStartSingleLoopPositive_ShapeOuterTimeRange<Cntx_t>,
     &build_IFSFromStartSingleLoopPositive_ShapeWindows<Cntx_t>, &check_IFSFromStartSingleLoopPositive_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSSynopticSingleLoop, "IFSSynopticSingleLoop", &match_IFSSynopticSingleLoop_Shape<Cntx_t>,
     &build_IFSSynopticSingleLoop_ShapeOuterTimeRange<Cntx_t>, &build_IFSSynopticSingleLoop_ShapeWindows<Cntx_t>,
     &check_IFSSynopticSingleLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::AIFSStandardSingleLoop, "AIFSStandardSingleLoop", &match_AIFSStandardSingleLoop_Shape<Cntx_t>,
     &build_AIFSStandardSingleLoop_ShapeOuterTimeRange<Cntx_t>, &build_AIFSStandardSingleLoop_ShapeWindows<Cntx_t>,
     &check_AIFSStandardSingleLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop, "AIFSFakeDoubleLoopSingleLoop",
     &match_AIFSFakeDoubleLoopSingleLoop_Shape<Cntx_t>, &build_AIFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange<Cntx_t>,
     &build_AIFSFakeDoubleLoopSingleLoop_ShapeWindows<Cntx_t>, &check_AIFSFakeDoubleLoopSingleLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::AIFSFromStartSingleLoopAtZero, "AIFSFromStartSingleLoopAtZero",
     &match_AIFSFromStartSingleLoopAtZero_Shape<Cntx_t>, &build_AIFSFromStartSingleLoopAtZero_ShapeOuterTimeRange<Cntx_t>,
     &build_AIFSFromStartSingleLoopAtZero_ShapeWindows<Cntx_t>, &check_AIFSFromStartSingleLoopAtZero_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive, "AIFSFromStartSingleLoopPositive",
     &match_AIFSFromStartSingleLoopPositive_Shape<Cntx_t>, &build_AIFSFromStartSingleLoopPositive_ShapeOuterTimeRange<Cntx_t>,
     &build_AIFSFromStartSingleLoopPositive_ShapeWindows<Cntx_t>, &check_AIFSFromStartSingleLoopPositive_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::SeasonalSingleLoop, "SeasonalSingleLoop", &match_SeasonalSingleLoop_Shape<Cntx_t>,
     &build_SeasonalSingleLoop_ShapeOuterTimeRange<Cntx_t>, &build_SeasonalSingleLoop_ShapeWindows<Cntx_t>,
     &check_SeasonalSingleLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::SeasonalMultiloop, "SeasonalMultiloop", &match_SeasonalMultiloop_Shape<Cntx_t>,
     &build_SeasonalMultiloop_ShapeOuterTimeRange<Cntx_t>, &build_SeasonalMultiloop_ShapeWindows<Cntx_t>,
     &check_SeasonalMultiloop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSStandardMultiLoop, "IFSStandardMultiLoop", &match_IFSStandardMultiLoop_Shape<Cntx_t>,
     &build_IFSStandardMultiLoop_ShapeOuterTimeRange<Cntx_t>, &build_IFSStandardMultiLoop_ShapeWindows<Cntx_t>,
     &check_IFSStandardMultiLoop_Shape<Cntx_t>},
    {ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop, "IFSFakeSingleLoopDoubleLoop",
     &match_IFSFakeSingleLoopDoubleLoop_Shape<Cntx_t>, &build_IFSFakeSingleLoopDoubleLoop_ShapeOuterTimeRange<Cntx_t>,
     &build_IFSFakeSingleLoopDoubleLoop_ShapeWindows<Cntx_t>, &check_IFSFakeSingleLoopDoubleLoop_Shape<Cntx_t>},
}};

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
template <class Cntx_t>
inline ProductTimeSpecShapeKind classify_Shape_or_throw(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const auto& cases = detail::shapeCases<Cntx_t>;
        std::array<bool, static_cast<std::size_t>(ProductTimeSpecShapeKind::Count)> matches{};
        std::size_t numberOfMatches = 0;
        std::size_t matchedIndex    = 0;

        for (std::size_t i = 0; i < cases.size(); ++i) {
            matches[i] = cases[i].matcher(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

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
                        << " matches. Match results: ";
                    oss << "{param=" << input.marsParamId << ", class=" << input.marsClass
                        << ", stream=" << input.marsStream << ", type=" << input.marsType << "}, [";
                    for (std::size_t i = 0; i < matches.size(); ++i) {
                        oss << cases[i].name << "=" << (matches[i] ? "true" : "false");
                        if (i < matches.size() - 1) {
                            oss << ", ";
                        }
                    }
                    oss << "]";
                    return oss.str();
                }(),
                input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            ProductTimeSpecShapeKind result = cases[matchedIndex].classification;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to classify the ProductTimeSpec shape", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
template <class Cntx_t>
inline ProductTimeSpecOuterTimeRange build_ShapeOuterTimeRange_or_throw(
    ProductTimeSpecShapeKind classification, const ProductTimeSpecInput& input,
    const ProductTimeSpecClassification& fullClassification, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const auto& cases                 = detail::shapeCases<Cntx_t>;
        const bool classificationIsValid = index < cases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecShapeKind value", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            ProductTimeSpecOuterTimeRange result = cases[index].outerTimeRangeBuilder(input, fullClassification, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec outer time range", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
template <class Cntx_t>
inline ProductTimeSpecShape build_ShapeWindows_or_throw(ProductTimeSpecShapeKind classification,
                                                        const ProductTimeSpecInput& input,
                                                        const ProductTimeSpecClassification& classificationBundle,
                                                        const anchor::ProductTimeSpecAnchor& anchor,
                                                        const ProductTimeSpecOuterTimeRange& outerTimeRange,
                                                        const domain::ProductTimeSpecDomain& domain, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const auto& cases                 = detail::shapeCases<Cntx_t>;
        const bool classificationIsValid = index < cases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecShapeKind value", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            ProductTimeSpecShape result = cases[index].windowsBuilder(input, classificationBundle, anchor, outerTimeRange, domain, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec windows", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

///
/// @brief Dispatch the checker associated with a validated shape classification.
///
/// @param[in] classification
/// Unique shape classification returned by `classify_Shape_or_throw`.
/// @param[in] input Fully normalized ProductTimeSpec input, including embedded options.
/// @param[in] classificationBundle Full resolved ProductTimeSpec classification bundle.
/// @param[in] anchor Resolved ProductTimeSpec anchor.
/// @param[in] outerTimeRange Stage-1 ProductTimeSpec outer time range artifact.
/// @param[in] domain Resolved ProductTimeSpec domain.
/// @param[in] shape Complete ProductTimeSpec shape artifact produced by the selected final builder.
/// @return `true` when the selected checker validates the shape successfully.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException If the classification is invalid or the
///         selected checker fails.
///
template <class Cntx_t>
inline bool check_Shape_or_throw(ProductTimeSpecShapeKind classification, const ProductTimeSpecInput& input,
                                 const ProductTimeSpecClassification& classificationBundle,
                                 const anchor::ProductTimeSpecAnchor& anchor,
                                 const ProductTimeSpecOuterTimeRange& outerTimeRange,
                                 const domain::ProductTimeSpecDomain& domain, const ProductTimeSpecShape& shape, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const auto& cases                 = detail::shapeCases<Cntx_t>;
        const bool classificationIsValid = index < cases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecShapeKind value", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            bool result = cases[index].checker(input, classificationBundle, anchor, outerTimeRange, domain, shape, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to check the ProductTimeSpec shape", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape

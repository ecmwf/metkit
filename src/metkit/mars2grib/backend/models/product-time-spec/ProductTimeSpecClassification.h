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
/// @file ProductTimeSpecClassification.h
/// @brief Shared ProductTimeSpec classification enums, helpers, and bundle.
///
/// This header centralizes the classification-only layer shared by anchors,
/// domains, shapes, and the top-level ProductTimeSpec orchestration. It owns:
/// - the anchor, shape, and domain classification enums;
/// - the best-effort `noexcept` enum-name helpers used by diagnostic code;
/// - the aggregate `ProductTimeSpecClassification` struct;
/// - the best-effort `noexcept` JSON helper for the full classification.
///
/// It intentionally does not define any resolved anchor, shape, or domain
/// artifacts, so header-only callback layers can depend on classifications
/// without introducing circular dependencies.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <cstddef>
#include <sstream>
#include <string>

#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"

namespace metkit::mars2grib::backend::models::product_time_spec {

namespace anchor {

///
/// @brief Supported temporal-anchor construction cases.
///
/// The active backend-model anchor cases are:
/// - `ForecastAnalysis`, for products carrying one direct MARS `date` source and
///   an optional MARS `time` source;
/// - `Hindcast`, for products carrying both direct MARS `date` and direct MARS
///   `hdate` sources;
/// - `SeasonalClimate`, for products carrying MARS `year` and `month` without
///   direct `date`, `time`, or `hdate` sources.
///
/// `Count` is a sentinel used exclusively to size the registry.
///
enum class ProductTimeSpecAnchorKind : std::size_t {
    ForecastAnalysis,
    Hindcast,
    SeasonalClimate,
    Count  // Used for sizing the registry, not a valid classification.
};

/// @brief Return the stable diagnostic name of one anchor case.
/// @param[in] value Anchor classification value.
/// @return Stable human-readable anchor-case name, or a fallback string.
inline std::string productTimeSpecAnchorTypeName(ProductTimeSpecAnchorKind value) noexcept {
    try {
        switch (value) {
            case ProductTimeSpecAnchorKind::ForecastAnalysis:
                return "ForecastAnalysis";
            case ProductTimeSpecAnchorKind::Hindcast:
                return "Hindcast";
            case ProductTimeSpecAnchorKind::SeasonalClimate:
                return "SeasonalClimate";
            case ProductTimeSpecAnchorKind::Count:
                return "Count";
        }

        return "InvalidTimeAnchorKind";
    }
    catch (...) {
        return "ProductTimeSpecAnchorKindNameError";
    }
}

}  // namespace anchor

namespace domain {

///
/// @brief Supported absolute-domain construction strategies.
///
/// The active backend-model domain cases are:
/// - `ForecastDomain`, for non-synoptic non-seasonal forecast products whose
///   support ends at `referenceDateTime + step` and extends backward by the
///   resolved outer range;
/// - `SeasonalForecastDomain`, for non-synoptic seasonal forecast products whose
///   support ends at `referenceDateTime + fcmonth months` and extends backward
///   by the resolved outer range;
/// - `AnalysisDomain`, for non-synoptic analysis products whose support starts
///   at `referenceDateTime` and extends forward by the resolved outer range;
/// - `SynopticAnalysisDomain`, for synoptic IFS analysis products whose support
///   starts at the exact MARS date/time and ends at the beginning of the next
///   calendar month.
///
/// `Count` is a sentinel used exclusively to size the registry.
///
enum class ProductTimeSpecDomainKind : std::size_t {
    ForecastDomain,
    SeasonalForecastDomain,
    AnalysisDomain,
    SynopticAnalysisDomain,
    Count
};

/// @brief Return the stable diagnostic name of one domain case.
/// @param[in] value Domain classification value.
/// @return Stable human-readable domain-case name, or a fallback string.
inline std::string productTimeSpecDomainTypeName(ProductTimeSpecDomainKind value) noexcept {
    try {
        switch (value) {
            case ProductTimeSpecDomainKind::ForecastDomain:
                return "ForecastDomain";
            case ProductTimeSpecDomainKind::SeasonalForecastDomain:
                return "SeasonalForecastDomain";
            case ProductTimeSpecDomainKind::AnalysisDomain:
                return "AnalysisDomain";
            case ProductTimeSpecDomainKind::SynopticAnalysisDomain:
                return "SynopticAnalysisDomain";
            case ProductTimeSpecDomainKind::Count:
                return "Count";
        }

        return "InvalidTimeDomainKind";
    }
    catch (...) {
        return "ProductTimeSpecDomainKindNameError";
    }
}

}  // namespace domain

namespace shape {

///
/// @brief Supported canonical ProductTimeSpec shape cases.
///
/// Shape classification describes the canonical statistical-window topology of
/// the final ProductTimeSpec representation.
///
/// The active cases are grouped as follows:
/// - instant-product representations;
/// - IFS single-loop and multi-loop statistical representations;
/// - AIFS single-loop statistical representations whose increment semantics are
///   always missing;
/// - seasonal forecast representations bound to the dedicated
///   `SeasonalForecastDomain`;
/// - from-start variants distinguished by zero-length versus positive-step
///   semantics;
/// - fake-loop compatibility cases that preserve legacy source encodings while
///   producing canonical windows.
///
/// `Count` is a sentinel used exclusively to size the registry.
///
enum class ProductTimeSpecShapeKind : std::size_t {
    Instant,

    IFSStandardSingleLoop,
    IFSFakeDoubleLoopSingleLoop,
    IFSFromStartSingleLoopAtZero,
    IFSFromStartSingleLoopPositive,
    IFSSynopticSingleLoop,

    AIFSStandardSingleLoop,
    AIFSFakeDoubleLoopSingleLoop,
    AIFSFromStartSingleLoopAtZero,
    AIFSFromStartSingleLoopPositive,

    SeasonalSingleLoop,
    SeasonalMultiloop,

    IFSStandardMultiLoop,
    IFSFakeSingleLoopDoubleLoop,

    Count  // Used for sizing the registry, not a valid classification.
};

/// @brief Return the stable diagnostic name of one shape case.
/// @param[in] value Shape classification value.
/// @return Stable human-readable shape-case name, or a fallback string.
inline std::string productTimeSpecShapeTypeName(ProductTimeSpecShapeKind value) noexcept {
    try {
        switch (value) {
            case ProductTimeSpecShapeKind::Instant:
                return "Instant";

            case ProductTimeSpecShapeKind::IFSStandardSingleLoop:
                return "IFSStandardSingleLoop";
            case ProductTimeSpecShapeKind::IFSFakeDoubleLoopSingleLoop:
                return "IFSFakeDoubleLoopSingleLoop";
            case ProductTimeSpecShapeKind::IFSFromStartSingleLoopAtZero:
                return "IFSFromStartSingleLoopAtZero";
            case ProductTimeSpecShapeKind::IFSFromStartSingleLoopPositive:
                return "IFSFromStartSingleLoopPositive";
            case ProductTimeSpecShapeKind::IFSSynopticSingleLoop:
                return "IFSSynopticSingleLoop";

            case ProductTimeSpecShapeKind::AIFSStandardSingleLoop:
                return "AIFSStandardSingleLoop";
            case ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop:
                return "AIFSFakeDoubleLoopSingleLoop";
            case ProductTimeSpecShapeKind::AIFSFromStartSingleLoopAtZero:
                return "AIFSFromStartSingleLoopAtZero";
            case ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive:
                return "AIFSFromStartSingleLoopPositive";

            case ProductTimeSpecShapeKind::SeasonalSingleLoop:
                return "SeasonalSingleLoop";
            case ProductTimeSpecShapeKind::SeasonalMultiloop:
                return "SeasonalMultiloop";

            case ProductTimeSpecShapeKind::IFSStandardMultiLoop:
                return "IFSStandardMultiLoop";
            case ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop:
                return "IFSFakeSingleLoopDoubleLoop";
            case ProductTimeSpecShapeKind::Count:
                return "Count";
        }

        return "InvalidShapeKind";
    }
    catch (...) {
        return "ProductTimeSpecShapeKindNameError";
    }
}

}  // namespace shape

///
/// @brief Full ProductTimeSpec classification bundle.
///
/// This aggregate keeps the resolved anchor, shape, and domain classifications
/// together so later build phases can carry one consistent classification object.
///
struct ProductTimeSpecClassification {
    anchor::ProductTimeSpecAnchorKind anchorType{anchor::ProductTimeSpecAnchorKind::ForecastAnalysis};
    shape::ProductTimeSpecShapeKind shapeType{shape::ProductTimeSpecShapeKind::Instant};
    domain::ProductTimeSpecDomainKind domainType{domain::ProductTimeSpecDomainKind::ForecastDomain};
};

/// @brief Serialize one resolved ProductTimeSpec classification as diagnostic JSON.
/// @param[in] value Resolved classification bundle.
/// @return One JSON object describing the classification, or a fallback error object.
inline std::string productTimeSpecClassificationJson(const ProductTimeSpecClassification& value) noexcept {
    try {
        std::ostringstream out;
        out << '{' << detail::jsonQuote_modelInput("anchorType") << ':'
            << detail::jsonQuote_modelInput(anchor::productTimeSpecAnchorTypeName(value.anchorType)) << ','
            << detail::jsonQuote_modelInput("shapeType") << ':'
            << detail::jsonQuote_modelInput(shape::productTimeSpecShapeTypeName(value.shapeType)) << ','
            << detail::jsonQuote_modelInput("domainType") << ':'
            << detail::jsonQuote_modelInput(domain::productTimeSpecDomainTypeName(value.domainType)) << '}';
        return out.str();
    }
    catch (...) {
        return std::string{
            "{\"error\":\"ProductTimeSpecClassificationJson failed while building diagnostic context\"}"};
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec

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
/// @file DomainRegistry.h
/// @brief Register, classify, and dispatch ProductTimeSpec domain cases.
///
/// Every domain matcher is evaluated independently. Classification succeeds only
/// when exactly one matcher returns `true`; matcher order never resolves overlap.
/// Zero matches identify unsupported semantics, while multiple matches identify
/// an error in matcher boundaries.
///
/// The active callback-selection matrix is:
///
/// | `isSynoptic` | `regime` | `simulationType` | Selected domain callback |
/// |--------------|----------|------------------|--------------------------|
/// | `false`      | any      | `Forecast`, non-seasonal, non-from-start | `ForecastDomain` |
/// | `false`      | any      | `Forecast`, non-seasonal, from-start | `FromStartForecastDomain` |
/// | `false`      | any      | `Forecast`, seasonal | `SeasonalForecastDomain` |
/// | `false`      | not `AIFS` | `Analysis`     | `AnalysisDomain`         |
/// | `true`       | `IFS`    | `Analysis`       | `SynopticAnalysisDomain` |
///
/// Unsupported combinations include, among others:
/// - synoptic forecast products;
/// - synoptic AIFS analysis products;
/// - non-synoptic AIFS analysis products.
///
/// @ingroup mars2grib_product_time_spec_domains
///
#pragma once

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/AnalysisDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/ForecastDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/FromStartForecastDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/SeasonalForecastDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/SynopticAnalysisDomain.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain {

namespace detail {

/// @brief Function-pointer type shared by all domain matchers.
using DomainMatcher = bool (*)(const ProductTimeSpecInput&);

///
/// @brief Function-pointer type shared by all domain builders.
///
/// A domain builder receives normalized input, the resolved classification
/// bundle, the already constructed anchor, and the stage-1 shape artifact. It
/// returns the complete absolute support interval.
///
using DomainBuilder = ProductTimeSpecDomain (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&,
                                                 const anchor::ProductTimeSpecAnchor&,
                                                 const shape::ProductTimeSpecOuterTimeRange&);

///
/// @brief Immutable registry row for one domain case.
///
/// The row keeps the classification value, diagnostic name, matcher, and builder
/// together so that independent arrays cannot become misaligned.
///
struct DomainCase {
    ProductTimeSpecDomainKind classification;
    std::string_view name;
    DomainMatcher matcher;
    DomainBuilder builder;
};

/// @brief Immutable domain registry ordered exactly like `ProductTimeSpecDomainKind`.
inline constexpr std::array<DomainCase, static_cast<std::size_t>(ProductTimeSpecDomainKind::Count)> domainCases{{
    {ProductTimeSpecDomainKind::ForecastDomain, "ForecastDomain", &match_Forecast_Domain, &build_Forecast_Domain},
    {ProductTimeSpecDomainKind::FromStartForecastDomain, "FromStartForecastDomain", &match_FromStartForecast_Domain,
     &build_FromStartForecast_Domain},
    {ProductTimeSpecDomainKind::SeasonalForecastDomain, "SeasonalForecastDomain", &match_SeasonalForecast_Domain,
     &build_SeasonalForecast_Domain},
    {ProductTimeSpecDomainKind::AnalysisDomain, "AnalysisDomain", &match_Analysis_Domain, &build_Analysis_Domain},
    {ProductTimeSpecDomainKind::SynopticAnalysisDomain, "SynopticAnalysisDomain", &match_SynopticAnalysis_Domain,
     &build_SynopticAnalysis_Domain},
}};

static_assert(static_cast<std::size_t>(detail::domainCases[0].classification) ==
              static_cast<std::size_t>(ProductTimeSpecDomainKind::ForecastDomain));
static_assert(static_cast<std::size_t>(detail::domainCases[1].classification) ==
              static_cast<std::size_t>(ProductTimeSpecDomainKind::FromStartForecastDomain));
static_assert(static_cast<std::size_t>(detail::domainCases[2].classification) ==
              static_cast<std::size_t>(ProductTimeSpecDomainKind::SeasonalForecastDomain));
static_assert(static_cast<std::size_t>(detail::domainCases[3].classification) ==
              static_cast<std::size_t>(ProductTimeSpecDomainKind::AnalysisDomain));
static_assert(static_cast<std::size_t>(detail::domainCases[4].classification) ==
              static_cast<std::size_t>(ProductTimeSpecDomainKind::SynopticAnalysisDomain));

}  // namespace detail

///
/// @brief Classify the normalized input against every registered domain case.
///
/// @section Domain classification contract
/// - Reads: `analysisOrForecast` and synoptic facts from normalized input.
/// - Evaluates: every matcher in `domainCases`.
/// - Success: exactly one matcher returns `true`.
/// - Failure: zero or multiple matchers return `true`.
/// - Side effects: none.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input.
///
/// @return
/// Unique matching `ProductTimeSpecDomainKind` value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If matcher evaluation fails or classification is not unique.
///
inline ProductTimeSpecDomainKind classify_Domain_or_throw(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        std::array<bool, detail::domainCases.size()> matches{};
        std::size_t numberOfMatches = 0;
        std::size_t matchedIndex    = 0;

        for (std::size_t i = 0; i < detail::domainCases.size(); ++i) {
            matches[i] = detail::domainCases[i].matcher(input);

            if (matches[i]) {
                ++numberOfMatches;
                matchedIndex = i;
            }
        }

        if (numberOfMatches != 1) {
            throw Mars2GribModelException("ProductTimeSpec domain classification is not unique or is unsupported (" +
                                              std::to_string(numberOfMatches) + " matches)",
                                          input.to_json(), Here());
        }

        return detail::domainCases[matchedIndex].classification;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to classify the ProductTimeSpec domain", input.to_json(), Here()));
    }
}

///
/// @brief Dispatch the builder associated with a validated domain classification.
///
/// @param[in] classification
/// Unique domain classification returned by `classify_Domain_or_throw`.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input.
///
/// @param[in] fullClassification Full resolved ProductTimeSpec classification bundle.
/// @param[in] anchor
/// Complete anchor constructed before domain construction.
/// @param[in] outerTimeRange
/// Stage-1 outer time range constructed before domain construction.
///
/// @return
/// Absolute ProductTimeSpec support interval.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If the classification is invalid or the selected builder fails.
///
inline ProductTimeSpecDomain build_Domain_or_throw(ProductTimeSpecDomainKind classification,
                                                    const ProductTimeSpecInput& input,
                                                     const ProductTimeSpecClassification& fullClassification,
                                                     const anchor::ProductTimeSpecAnchor& anchor,
                                                     const shape::ProductTimeSpecOuterTimeRange& outerTimeRange) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const bool classificationIsValid = index < detail::domainCases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecDomainKind value", input.to_json(), Here());
        }

        return detail::domainCases[index].builder(input, fullClassification, anchor, outerTimeRange);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec domain", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain

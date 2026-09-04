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
/// @brief Register, classify, build, and check ProductTimeSpec domain cases.
///
/// This header centralizes the case table and the public dispatch entry points
/// for ProductTimeSpec domain handling.
///
/// It owns:
/// - the immutable registry row type used to keep matcher, builder, and checker
///   callbacks aligned with their classification values;
/// - the domain classification entry point;
/// - the domain builder dispatch entry point;
/// - the domain checker dispatch entry point.
///
/// Every domain matcher is evaluated independently. Classification succeeds only
/// when exactly one matcher returns `true`; matcher order never resolves overlap.
/// Zero matches identify unsupported semantics, while multiple matches identify
/// overlap in matcher boundaries.
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
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/AnalysisDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/ForecastDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/FromStartForecastDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/SeasonalForecastDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/impl/SynopticAnalysisDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
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
/// bundle, the already constructed anchor, and the stage-1 outer time range. It
/// returns the complete raw absolute support interval.
///
using DomainBuilder = ProductTimeSpecDomain (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&,
                                                const anchor::ProductTimeSpecAnchor&,
                                                const shape::ProductTimeSpecOuterTimeRange&);

///
/// @brief Function-pointer type shared by all domain check callbacks.
using DomainChecker = bool (*)(const ProductTimeSpecInput&, const anchor::ProductTimeSpecAnchor&,
                               const ProductTimeSpecDomain&);

/// @brief Immutable registry row for one domain case.
///
/// The row keeps the classification value, diagnostic name, matcher, builder,
/// and checker together so that independent arrays cannot become misaligned.
///
struct DomainCase {
    ProductTimeSpecDomainKind classification;
    std::string_view name;
    DomainMatcher matcher;
    DomainBuilder builder;
    DomainChecker checker;
};

/// @brief Immutable domain registry ordered exactly like `ProductTimeSpecDomainKind`.
inline constexpr std::array<DomainCase, static_cast<std::size_t>(ProductTimeSpecDomainKind::Count)> domainCases{{
    {ProductTimeSpecDomainKind::ForecastDomain, "ForecastDomain", &match_Forecast_Domain, &build_Forecast_Domain,
     &check_Forecast_Domain},
    {ProductTimeSpecDomainKind::FromStartForecastDomain, "FromStartForecastDomain", &match_FromStartForecast_Domain,
     &build_FromStartForecast_Domain, &check_FromStartForecast_Domain},
    {ProductTimeSpecDomainKind::SeasonalForecastDomain, "SeasonalForecastDomain", &match_SeasonalForecast_Domain,
     &build_SeasonalForecast_Domain, &check_SeasonalForecast_Domain},
    {ProductTimeSpecDomainKind::AnalysisDomain, "AnalysisDomain", &match_Analysis_Domain, &build_Analysis_Domain,
     &check_Analysis_Domain},
    {ProductTimeSpecDomainKind::SynopticAnalysisDomain, "SynopticAnalysisDomain", &match_SynopticAnalysis_Domain,
     &build_SynopticAnalysis_Domain, &check_SynopticAnalysis_Domain},
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
/// - Reads: normalized synoptic, regime, simulation-type, and timespan facts.
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

///
/// @brief Dispatch the checker associated with a validated domain classification.
///
/// @param[in] classification
/// Unique domain classification returned by `classify_Domain_or_throw`.
///
/// @param[in] input
/// Fully normalized ProductTimeSpec input.
///
/// @param[in] anchor
/// Complete anchor constructed before domain validation.
///
/// @param[in] domain
/// Complete domain artifact produced by the selected domain builder.
///
/// @return
/// `true` when the selected checker validates the domain successfully.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
/// If the classification is invalid or the selected checker fails.
///
inline bool check_Domain_or_throw(ProductTimeSpecDomainKind classification, const ProductTimeSpecInput& input,
                                  const anchor::ProductTimeSpecAnchor& anchor, const ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const bool classificationIsValid = index < detail::domainCases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid ProductTimeSpecDomainKind value", input.to_json(), Here());
        }

        return detail::domainCases[index].checker(input, anchor, domain);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to check the ProductTimeSpec domain", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain

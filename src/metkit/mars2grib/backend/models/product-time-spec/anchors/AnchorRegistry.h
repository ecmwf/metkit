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
/// @file AnchorRegistry.h
/// @brief Register, classify, build, and check ProductTimeSpec anchor cases.
///
/// This header centralizes the case table and the public dispatch entry points
/// for ProductTimeSpec anchor handling.
///
/// It owns:
/// - the immutable registry row type used to keep matcher, builder, and checker
///   callbacks aligned with their classification values;
/// - the anchor classification entry point;
/// - the anchor builder dispatch entry point;
/// - the anchor checker dispatch entry point.
///
/// Classification is exhaustive and non-prioritized. Every registered matcher
/// is evaluated, and classification succeeds only when exactly one matcher
/// returns `true`.
///
/// A zero-match result means that the normalized input does not describe a
/// supported implemented anchor. A multiple-match result means that two or more
/// matcher contracts overlap. Both are hard classification failures.
///
/// The active callback-selection matrix is:
///
/// | MARS `date` | MARS `time` | MARS `hdate` | MARS `year` / `month` | Selected anchor callback |
/// |-------------|-------------|--------------|------------------------|--------------------------|
/// | present     | optional    | absent       | both absent            | `ForecastAnalysis`       |
/// | present     | optional    | present      | both absent            | `Hindcast`               |
/// | absent      | absent      | absent       | one or both present    | `SeasonalClimate` matcher raises `not
/// implemented` |
///
/// Unsupported source states include, among others:
/// - partial `year` / `month` presence;
/// - `time` without `date`;
/// - simultaneous direct `date` / `hdate` sources together with `year` or `month`;
/// - complete absence of all anchor source families.
///
/// @ingroup mars2grib_product_time_spec_anchors
///
#pragma once

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/impl/ForecastAnalysis.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/impl/Hindcast.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/impl/SeasonalClimate.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor {

namespace detail {

/// @brief Function-pointer type shared by all anchor matchers.
template <class Cntx_t>
using AnchorMatcher = bool (*)(const ProductTimeSpecInput&, Cntx_t&);

/// @brief Function-pointer type shared by all anchor builders.
template <class Cntx_t>
using AnchorBuilder = ProductTimeSpecAnchor (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&, Cntx_t&);

/// @brief Function-pointer type shared by all anchor check callbacks.
template <class Cntx_t>
using AnchorChecker = bool (*)(const ProductTimeSpecInput&, const ProductTimeSpecAnchor&, Cntx_t&);

///
/// @brief Immutable registry row for one anchor case.
///
/// The row keeps the classification value, diagnostic name, matcher, builder,
/// and checker together so that independent arrays cannot become misaligned.
///
template <class Cntx_t>
struct AnchorCase {
    ProductTimeSpecAnchorKind classification;
    std::string_view name;
    AnchorMatcher<Cntx_t> matcher;
    AnchorBuilder<Cntx_t> builder;
    AnchorChecker<Cntx_t> checker;
};

template <class Cntx_t>
inline constexpr std::array<detail::AnchorCase<Cntx_t>, static_cast<std::size_t>(ProductTimeSpecAnchorKind::Count)> anchorCases{
    {
        {ProductTimeSpecAnchorKind::ForecastAnalysis, "ForecastAnalysis", &match_ForecastAnalysis_Anchor<Cntx_t>,
         &build_ForecastAnalysis_Anchor<Cntx_t>, &check_ForecastAnalysis_Anchor<Cntx_t>},
        {ProductTimeSpecAnchorKind::Hindcast, "Hindcast", &match_Hindcast_Anchor<Cntx_t>,
         &build_Hindcast_Anchor<Cntx_t>, &check_Hindcast_Anchor<Cntx_t>},
        {ProductTimeSpecAnchorKind::SeasonalClimate, "SeasonalClimate", &match_SeasonalClimate_Anchor<Cntx_t>,
         &build_SeasonalClimate_Anchor<Cntx_t>, &check_SeasonalClimate_Anchor<Cntx_t>},
    }};

}  // namespace detail

///
/// @brief Classify the normalized input against every registered anchor case.
///
/// @section Anchor classification contract
/// - Reads: normalized direct-source facts from `input`.
/// - Evaluates: every matcher in `anchorCases`.
/// - Success: exactly one matcher returns `true`.
/// - Failure: zero or more than one matcher return `true`.
/// - Side effects: none.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return Unique matching `ProductTimeSpecAnchorKind` value.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
///         if matcher evaluation fails or classification is not unique.
///
template <class Cntx_t>
inline ProductTimeSpecAnchorKind classify_Anchor_or_throw(const ProductTimeSpecInput& input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const auto& cases = detail::anchorCases<Cntx_t>;
        std::array<bool, static_cast<std::size_t>(ProductTimeSpecAnchorKind::Count)> matches{};
        std::size_t numberOfMatches = 0;
        std::size_t matchedIndex    = 0;

        if (!cases.empty()) {
            for (std::size_t i = 0; i < cases.size(); ++i) {
                matches[i] = cases[i].matcher(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
                if (matches[i]) {
                    ++numberOfMatches;
                    matchedIndex = i;
                }
            }
        }

        if (numberOfMatches != 1) {
            throw Mars2GribModelException("Anchor classification failed: expected exactly one match, but found " +
                                              std::to_string(numberOfMatches) + " matches",
                                          input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            ProductTimeSpecAnchorKind result = cases[matchedIndex].classification;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to classify the ProductTimeSpec anchor", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

///
/// @brief Dispatch the builder associated with a validated anchor classification.
///
/// @param[in] classification Unique anchor classification returned by
///            `classify_Anchor_or_throw`.
/// @param[in] input Fully normalized ProductTimeSpec input supplied to the
///            selected anchor builder.
/// @param[in] fullClassification Full resolved ProductTimeSpec classification bundle.
/// @return Complete and case-validated `ProductTimeSpecAnchor`.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if
///         the classification is invalid or the selected builder fails.
///
template <class Cntx_t>
inline ProductTimeSpecAnchor build_Anchor_or_throw(ProductTimeSpecAnchorKind classification,
                                                   const ProductTimeSpecInput& input,
                                                   const ProductTimeSpecClassification& fullClassification, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const auto& cases                 = detail::anchorCases<Cntx_t>;
        const bool classificationIsValid = index < cases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid AnchorClassification value", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            ProductTimeSpecAnchor result = cases[index].builder(input, fullClassification, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec anchor", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

///
/// @brief Dispatch the checker associated with a validated anchor classification.
///
/// @param[in] classification Unique anchor classification returned by
///            `classify_Anchor_or_throw`.
/// @param[in] input Fully normalized ProductTimeSpec input supplied to the
///            selected anchor checker.
/// @param[in] anchor Complete anchor artifact produced by the selected anchor builder.
/// @return `true` when the selected checker validates the anchor successfully.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if
///         the classification is invalid or the selected checker fails.
///
template <class Cntx_t>
inline bool check_Anchor_or_throw(ProductTimeSpecAnchorKind classification, const ProductTimeSpecInput& input,
                                  const ProductTimeSpecAnchor& anchor, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const auto& cases                 = detail::anchorCases<Cntx_t>;
        const bool classificationIsValid = index < cases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid AnchorClassification value", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here());
        }

        {
            bool result = cases[index].checker(input, anchor, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to check the ProductTimeSpec anchor", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor

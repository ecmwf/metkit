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
/// @brief Register, classify, and dispatch ProductTimeSpec anchor cases.
///
/// The registry evaluates every anchor matcher and stores every Boolean result.
/// Matcher order is not a precedence mechanism: classification succeeds only
/// when exactly one matcher returns `true`.
///
/// A zero-match result means that the normalized input does not describe a
/// supported anchor. A multiple-match result means that two or more matcher
/// contracts overlap. Both are hard classification failures and include the
/// complete matcher-result vector in the diagnostic.
///
/// The active callback-selection matrix is:
///
/// | MARS `date` | MARS `time` | MARS `hdate` | MARS `year` / `month` | Selected anchor callback |
/// |-------------|-------------|--------------|------------------------|--------------------------|
/// | present     | optional    | absent       | both absent            | `ForecastAnalysis`       |
/// | present     | optional    | present      | both absent            | `Hindcast`               |
/// | absent      | absent      | absent       | both present           | `SeasonalClimate`        |
///
/// Unsupported source states include, among others:
/// - partial `year` / `month` presence;
/// - `time` without `date`;
/// - simultaneous direct `date` / `hdate` sources together with `year` / `month`;
/// - complete absence of all anchor source families.
///
/// @ingroup mars2grib_product_time_spec_anchors
///
#pragma once

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
using AnchorMatcher = bool (*)(const ProductTimeSpecInput&);

/// @brief Function-pointer type shared by all anchor builders.
using AnchorBuilder = ProductTimeSpecAnchor (*)(const ProductTimeSpecInput&, const ProductTimeSpecClassification&);

///
/// @brief Immutable registry row for one anchor case.
///
/// The row keeps the classification value, diagnostic name, matcher, and builder
/// together so that independent arrays cannot become misaligned.
///
struct AnchorCase {
    ProductTimeSpecAnchorKind classification;
    std::string_view name;
    AnchorMatcher matcher;
    AnchorBuilder builder;
};

inline constexpr std::array<detail::AnchorCase, static_cast<std::size_t>(ProductTimeSpecAnchorKind::Count)> anchorCases{
    {
        {ProductTimeSpecAnchorKind::ForecastAnalysis, "ForecastAnalysis", &match_ForecastAnalysis_Anchor,
         &build_ForecastAnalysis_Anchor},
        {ProductTimeSpecAnchorKind::Hindcast, "Hindcast", &match_Hindcast_Anchor, &build_Hindcast_Anchor},
        {ProductTimeSpecAnchorKind::SeasonalClimate, "SeasonalClimate", &match_SeasonalClimate_Anchor,
         &build_SeasonalClimate_Anchor},
    }};

static_assert(static_cast<std::size_t>(detail::anchorCases[0].classification) ==
              static_cast<std::size_t>(ProductTimeSpecAnchorKind::ForecastAnalysis));
static_assert(static_cast<std::size_t>(detail::anchorCases[1].classification) ==
              static_cast<std::size_t>(ProductTimeSpecAnchorKind::Hindcast));
static_assert(static_cast<std::size_t>(detail::anchorCases[2].classification) ==
              static_cast<std::size_t>(ProductTimeSpecAnchorKind::SeasonalClimate));

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
///         If matcher evaluation fails or classification is not unique.
///
inline ProductTimeSpecAnchorKind classify_Anchor_or_throw(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        std::array<bool, detail::anchorCases.size()> matches{};
        std::size_t numberOfMatches = 0;
        std::size_t matchedIndex    = 0;

        if (!detail::anchorCases.empty()) {
            for (std::size_t i = 0; i < detail::anchorCases.size(); ++i) {
                matches[i] = detail::anchorCases[i].matcher(input);
                if (matches[i]) {
                    ++numberOfMatches;
                    matchedIndex = i;
                }
            }
        }

        if (numberOfMatches != 1) {
            throw Mars2GribModelException("Anchor classification failed: expected exactly one match, but found " +
                                              std::to_string(numberOfMatches) + " matches",
                                          input.to_json(), Here());
        }

        return detail::anchorCases[matchedIndex].classification;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to classify the ProductTimeSpec anchor", input.to_json(), Here()));
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
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException If
///         the classification is invalid or the selected builder fails.
///
inline ProductTimeSpecAnchor build_Anchor_or_throw(ProductTimeSpecAnchorKind classification,
                                                    const ProductTimeSpecInput& input,
                                                    const ProductTimeSpecClassification& fullClassification) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::size_t index          = static_cast<std::size_t>(classification);
        const bool classificationIsValid = index < detail::anchorCases.size();

        if (!classificationIsValid) {
            throw Mars2GribModelException("Invalid AnchorClassification value", input.to_json(), Here());
        }

        return detail::anchorCases[index].builder(input, fullClassification);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build the ProductTimeSpec anchor", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor

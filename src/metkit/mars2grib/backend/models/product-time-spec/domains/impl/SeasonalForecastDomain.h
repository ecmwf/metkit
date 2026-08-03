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
/// @file SeasonalForecastDomain.h
/// @brief Matcher and builder for the seasonal forecast domain.
///
/// This header is the authoritative implementation of the
/// `SeasonalForecastDomain` domain case. The matcher identifies the absolute-
/// domain semantics, while the builder constructs `domainStartDateTime` and
/// `domainEndDateTime` from the normalized input and the resolved anchor.
///
/// The complete high-level domain rule remains visible in this file. Only
/// common temporal arithmetic and normalized-value extraction are delegated.
///
/// Every function catches all failures and rethrows a nested
/// `Mars2GribModelException` with the serialized input state.
///
/// @ingroup mars2grib_product_time_spec_domains
///

#pragma once

#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ForecastLeadUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainUtils.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail {

/**
 * @brief Return true only when input matches the seasonal forecast domain.
 *
 * - the product is not synoptic;
 * - the normalized input is seasonal;
 * - MARS semantics classify the product as forecast.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the domain matcher fails unexpectedly.
 */
inline bool match_SeasonalForecast_Domain(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isNotSynoptic = !input.isSynoptic;
        const bool isSeasonal    = product_time_spec::detail::isSeasonal(input);
        const bool isForecast    = input.simulationType == SimulationType::Forecast;

        return isNotSynoptic && isSeasonal && isForecast;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `match_SeasonalForecast_Domain`",
                                                       input.to_json(), Here()));
    }
}

/**
 * @brief End the domain at reference plus seasonal forecast lead and extend backward by the outer range.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @return Constructed ProductTimeSpec domain for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecDomain build_SeasonalForecast_Domain(const ProductTimeSpecInput& input,
                                                           const anchor::ProductTimeSpecAnchor& anchor) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::addDuration;
    using metkit::mars2grib::utils::time_arithmetic::subtractDuration;

    try {
        const auto forecastLead      = detail::resolvedSeasonalForecastLead(input);
        const auto domainEndDateTime = addDuration(anchor.referenceDateTime, forecastLead);
        const auto outerRange        = detail::resolveSeasonalForecastOuterDomainRange(input);
        return ProductTimeSpecDomain{subtractDuration(domainEndDateTime, outerRange), domainEndDateTime};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to execute `build_SeasonalForecast_Domain`",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

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
/// @file FromStartForecastDomain.h
/// @brief Matcher and builder for the non-seasonal from-start forecast domain.
///
/// This header isolates the forecast-domain path used by current from-start
/// shapes. The builder preserves the existing domain placement semantics while
/// allowing final shape construction to remain deferred until the domain is
/// available.
///
/// @ingroup mars2grib_product_time_spec_domains
///
#pragma once

#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ForecastLeadUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail {

inline bool match_FromStartForecast_Domain(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isNotSynoptic         = !input.isSynoptic;
        const bool isNotSeasonal         = !product_time_spec::detail::isSeasonal(input);
        const bool isForecast            = input.simulationType == SimulationType::Forecast;
        const bool usesFromStartTimespan = input.timespan.kind == TimespanKind::FromStart;

        return isNotSynoptic && isNotSeasonal && isForecast && usesFromStartTimespan;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_FromStartForecast_Domain`", input.to_json(), Here()));
    }
}

inline ProductTimeSpecDomain build_FromStartForecast_Domain(const ProductTimeSpecInput& input,
                                                            const ProductTimeSpecClassification& classification,
                                                            const anchor::ProductTimeSpecAnchor& anchor,
                                                            const shape::ProductTimeSpecOuterTimeRange& outerTimeRange) {
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::addDuration;
    using metkit::mars2grib::utils::time_arithmetic::subtractDuration;

    try {
        (void)classification;

        const bool outerTimeRangeIsDeferred =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Deferred;

        if (!outerTimeRangeIsDeferred || outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("FromStartForecastDomain requires a deferred outer time range",
                                          input.to_json(), Here());
        }

        const auto forecastLead      = detail::resolvedForecastStep(input);
        const auto domainEndDateTime = addDuration(anchor.referenceDateTime, forecastLead);
        const auto outerRange        = detail::resolvedForecastStep(input);
        const auto domainStartDateTime = subtractDuration(domainEndDateTime, outerRange);
        const bool isSynoptic          = false;
        const long startOffsetHoursFromReference = offsetHoursFromReference(anchor.referenceDateTime,
                                                                            domainStartDateTime);
        const long endOffsetHoursFromReference = offsetHoursFromReference(anchor.referenceDateTime,
                                                                          domainEndDateTime);

        return ProductTimeSpecDomain{domainStartDateTime, domainEndDateTime, isSynoptic,
                                     startOffsetHoursFromReference, endOffsetHoursFromReference};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_FromStartForecast_Domain`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

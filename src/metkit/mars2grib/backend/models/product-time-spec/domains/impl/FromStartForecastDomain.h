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
/// @brief Matcher, builder, and checker for the non-seasonal from-start forecast domain.
///
/// This header is the authoritative implementation of the
/// `FromStartForecastDomain` domain case. It keeps recognition, construction,
/// and validation together so that the complete case can be reviewed without
/// following a dispatch chain.
///
/// The matcher identifies the absolute-domain semantics. The builder constructs
/// all raw domain members directly from the resolved anchor and the from-start
/// forecast lead, while preserving the deferred outer-range contract used by the
/// later shape stage. The checker validates that the resolved domain remains
/// consistent with both the case semantics and the originating normalized input.
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

/**
 * @brief Return true only when input matches the non-seasonal from-start forecast domain.
 *
 * The case matches when:
 * - the product is not synoptic;
 * - the product does not satisfy both the seasonal class/stream discriminator
 *   and the seasonal lead discriminator;
 * - the simulation type is forecast;
 * - the source `timespan` uses from-start semantics.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the domain matcher fails unexpectedly.
 */
inline bool match_FromStartForecast_Domain(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::backend::deductions::TimespanKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isNotSynoptic = !input.isSynoptic;
        const bool hasSeasonalClassStream =
            (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
            (input.marsStream == "sfmd" || input.marsStream == "shmd");
        const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
        const bool isNotSeasonal            = !(hasSeasonalClassStream && hasSeasonalLeadSemantics);
        const bool isForecast               = input.simulationType == SimulationType::Forecast;
        const bool usesFromStartTimespan    = input.timespan.kind == TimespanKind::FromStart;

        return isNotSynoptic && isNotSeasonal && isForecast && usesFromStartTimespan;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_FromStartForecast_Domain`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the raw from-start forecast domain.
 *
 * In this case:
 * - the outer time range must remain deferred for the later shape stage;
 * - the real support end is the anchor reference datetime plus the resolved
 *   non-seasonal forecast lead;
 * - the real support start is the support end minus that same forecast lead;
 * - therefore the real support start resolves back to the anchor reference
 *   datetime;
 * - the domain is not synoptic;
 * - the hour offsets are measured from the anchor reference datetime.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
 * @return Constructed ProductTimeSpec domain for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecDomain build_FromStartForecast_Domain(
    const ProductTimeSpecInput& input, const ProductTimeSpecClassification& classification,
    const anchor::ProductTimeSpecAnchor& anchor, const shape::ProductTimeSpecOuterTimeRange& outerTimeRange) {
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
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

        if (!input.step.has_value()) {
            throw Mars2GribModelException("FromStartForecastDomain construction requires a resolved step",
                                          input.to_json(), Here());
        }

        // The support end is the anchor reference datetime extended by the
        // resolved non-seasonal forecast lead.
        const auto forecastLead      = *input.step;
        const auto domainEndDateTime = addDuration(anchor.referenceDateTime, forecastLead);

        // In the current from-start forecast semantics, the support start is
        // reconstructed by subtracting the same lead from the support end.
        const auto outerRange          = *input.step;
        const auto domainStartDateTime = subtractDuration(domainEndDateTime, outerRange);

        // This domain case is never synoptic.
        const bool isSynoptic = false;

        // The start offset is measured from the reference datetime to the real
        // support start. For the current from-start semantics, this should
        // resolve back to zero.
        const long startOffsetHoursFromReference =
            offsetHoursFromReference(anchor.referenceDateTime, domainStartDateTime);

        // The end offset is measured from the reference datetime to the real
        // support end.
        const long endOffsetHoursFromReference = offsetHoursFromReference(anchor.referenceDateTime, domainEndDateTime);

        return ProductTimeSpecDomain{domainStartDateTime, domainEndDateTime, isSynoptic, startOffsetHoursFromReference,
                                     endOffsetHoursFromReference};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_FromStartForecast_Domain`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved FromStartForecastDomain against its source input and anchor.
 *
 * This checker verifies:
 * - the domain is not synoptic;
 * - the support start does not follow the support end;
 * - the support start resolves to the anchor reference datetime under the
 *   current from-start semantics;
 * - the recorded hour offsets agree with the resolved start and end datetimes.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] domain Resolved domain artifact produced by the builder.
 * @return `true` when the domain is valid for the FromStartForecastDomain case.
 * @throws Mars2GribModelException if the resolved domain is inconsistent with
 *         the input, anchor, or case semantics.
 */
inline bool check_FromStartForecast_Domain(const ProductTimeSpecInput& input,
                                           const anchor::ProductTimeSpecAnchor& anchor,
                                           const ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        if (domain.isSynoptic) {
            throw Mars2GribModelException("FromStartForecastDomain must not be synoptic", input.to_json(), Here());
        }

        if (domain.domainStartDateTime > domain.domainEndDateTime) {
            throw Mars2GribModelException("FromStartForecastDomain start must not follow domain end", input.to_json(),
                                          Here());
        }

        if (domain.domainStartDateTime != anchor.referenceDateTime) {
            throw Mars2GribModelException("FromStartForecastDomain start must equal anchor reference datetime",
                                          input.to_json(), Here());
        }

        if (domain.startOffsetHoursFromReference !=
            offsetHoursFromReference(anchor.referenceDateTime, domain.domainStartDateTime)) {
            throw Mars2GribModelException(
                "FromStartForecastDomain start offset does not match resolved datetime placement", input.to_json(),
                Here());
        }

        if (domain.endOffsetHoursFromReference !=
            offsetHoursFromReference(anchor.referenceDateTime, domain.domainEndDateTime)) {
            throw Mars2GribModelException(
                "FromStartForecastDomain end offset does not match resolved datetime placement", input.to_json(),
                Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_FromStartForecast_Domain`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

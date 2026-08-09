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
/// @file SynopticAnalysisDomain.h
/// @brief Matcher and builder for the synoptic analysis domain.
///
/// This header is the authoritative implementation of the `SynopticAnalysisDomain` domain case. The matcher identifies
/// the absolute-domain semantics, while the builder constructs `domainStartDateTime` and `domainEndDateTime` from the
/// normalized input and the resolved anchor.
///
/// The complete high-level domain rule remains visible in this file. Only common temporal arithmetic and
/// normalized-value extraction are delegated.
///
/// Every function catches all failures and rethrows a nested `Mars2GribModelException` with the serialized input state.
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
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail {

/**
 * @brief Return true only when input matches the synoptic analysis domain.
 *
 * - the MARS product is synoptic;
 * - the regime is IFS;
 * - the product is an analysis.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the domain matcher fails unexpectedly.
 */
inline bool match_SynopticAnalysis_Domain(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isSynoptic = input.isSynoptic;
        const bool isIfs      = input.regime == SimulationRegime::IFS;
        const bool isAnalysis = input.simulationType == SimulationType::Analysis;

        return isSynoptic && isIfs && isAnalysis;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SynopticAnalysis_Domain`", input.to_json(), Here()));
    }
}

/**
 * @brief Preserve exact MARS date/time as the start and align the end to the next month boundary.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] shapeStage1 Previously constructed stage-1 ProductTimeSpec shape.
 * @return Constructed ProductTimeSpec domain for this unique case.
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecDomain build_SynopticAnalysis_Domain(const ProductTimeSpecInput& input,
                                                           const ProductTimeSpecClassification& classification,
                                                           const anchor::ProductTimeSpecAnchor& anchor,
                                                           const shape::ProductTimeSpecOuterTimeRange& outerTimeRange) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::beginningOfNextCalendarMonth;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        (void)classification;
        (void)anchor;

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("SynopticAnalysisDomain requires an available outer time range",
                                          input.to_json(), Here());
        }

        const TimeDuration expectedOuterTimeRange{1, TimeUnit::Month};

        if (outerTimeRange.timeRange->length != expectedOuterTimeRange.length ||
            outerTimeRange.timeRange->unit != expectedOuterTimeRange.unit) {
            throw Mars2GribModelException("SynopticAnalysisDomain requires a one-month outer time range",
                                          input.to_json(), Here());
        }

        if (!input.marsDate.has_value()) {
            throw Mars2GribModelException("Synoptic analysis domain requires an explicit MARS date", input.to_json(),
                                          Here());
        }
        const auto domainStartDateTime     = makeDateTime(*input.marsDate, input.marsTime);
        const auto realDomainStartDateTime = makeDateTime(domainStartDateTime.date(), defaultMarsTime());
        const auto domainEndDateTime       = beginningOfNextCalendarMonth(realDomainStartDateTime);
        const bool isSynoptic              = true;
        const long startOffsetHoursFromReference =
            offsetHoursFromReference(anchor.referenceDateTime, realDomainStartDateTime);
        const long endOffsetHoursFromReference = offsetHoursFromReference(anchor.referenceDateTime, domainEndDateTime);

        return ProductTimeSpecDomain{domainStartDateTime, domainEndDateTime, isSynoptic, startOffsetHoursFromReference,
                                     endOffsetHoursFromReference};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_SynopticAnalysis_Domain`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

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
/// @brief Matcher, builder, and checker for the synoptic analysis domain.
///
/// This header is the authoritative implementation of the
/// `SynopticAnalysisDomain` domain case. It keeps recognition, construction,
/// and validation together so that the complete case can be reviewed without
/// following a dispatch chain.
///
/// The matcher identifies the absolute-domain semantics. The builder constructs
/// all raw domain members directly from the resolved anchor and the synoptic
/// source datetime, while preserving the distinction between the stored
/// synoptic timestamp and the real support start at midnight. The checker
/// validates that the resolved domain remains consistent with both the case
/// semantics and the originating normalized input.
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
 * @brief Construct the raw synoptic analysis domain.
 *
 * In this case:
 * - the stored `domainStartDateTime` preserves the exact MARS date/time and
 *   therefore retains the synoptic hour;
 * - the real support start used for offsets is the same date forced to
 *   `00:00:00`;
 * - the support end is the first instant of the following calendar month
 *   computed from that real midnight-based support start;
 * - the domain is synoptic;
 * - the hour offsets are measured from the anchor reference datetime to the
 *   real support start and support end.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] outerTimeRange Previously constructed stage-1 outer time range.
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
        // The stored domain start preserves the exact MARS date/time and
        // therefore carries the synoptic hour in its time component.
        const auto domainStartDateTime = makeDateTime(*input.marsDate, input.marsTime);

        // The real support start used for support placement and hour offsets is
        // the same date forced to midnight.
        const auto realDomainStartDateTime = makeDateTime(domainStartDateTime.date(), defaultMarsTime());

        // The support end is the first instant of the following calendar month
        // computed from the real midnight-based support start.
        const auto domainEndDateTime = beginningOfNextCalendarMonth(realDomainStartDateTime);

        // This domain case is explicitly synoptic.
        const bool isSynoptic = true;

        // The start offset is measured from the reference datetime to the real
        // support start, not to the stored synoptic timestamp.
        const long startOffsetHoursFromReference =
            offsetHoursFromReference(anchor.referenceDateTime, realDomainStartDateTime);

        // The end offset is measured from the reference datetime to the real
        // support end.
        const long endOffsetHoursFromReference = offsetHoursFromReference(anchor.referenceDateTime, domainEndDateTime);

        return ProductTimeSpecDomain{domainStartDateTime, domainEndDateTime, isSynoptic, startOffsetHoursFromReference,
                                     endOffsetHoursFromReference};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_SynopticAnalysis_Domain`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved SynopticAnalysisDomain against its source input and anchor.
 *
 * This checker verifies:
 * - the domain is synoptic;
 * - the stored domain start date matches the input MARS date;
 * - the support start does not follow the support end;
 * - the support end is the next calendar-month boundary computed from the real
 *   midnight-based support start;
 * - the recorded hour offsets agree with the resolved real support start and
 *   support end.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] domain Resolved domain artifact produced by the builder.
 * @return `true` when the domain is valid for the SynopticAnalysisDomain case.
 * @throws Mars2GribModelException if the resolved domain is inconsistent with
 *         the input, anchor, or case semantics.
 */
inline bool check_SynopticAnalysis_Domain(const ProductTimeSpecInput& input,
                                          const anchor::ProductTimeSpecAnchor& anchor,
                                          const ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::beginningOfNextCalendarMonth;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        if (!domain.isSynoptic) {
            throw Mars2GribModelException("SynopticAnalysisDomain must be synoptic", input.to_json(), Here());
        }

        if (!input.marsDate.has_value()) {
            throw Mars2GribModelException("Input missing MARS date for SynopticAnalysisDomain", input.to_json(),
                                          Here());
        }

        if (domain.domainStartDateTime.date() != *input.marsDate) {
            throw Mars2GribModelException("SynopticAnalysisDomain start date does not match input MARS date",
                                          input.to_json(), Here());
        }

        if (domain.domainStartDateTime > domain.domainEndDateTime) {
            throw Mars2GribModelException("SynopticAnalysisDomain start must not follow domain end", input.to_json(),
                                          Here());
        }

        const eckit::DateTime realDomainStartDateTime =
            makeDateTime(domain.domainStartDateTime.date(), defaultMarsTime());

        if (domain.domainEndDateTime != beginningOfNextCalendarMonth(realDomainStartDateTime)) {
            throw Mars2GribModelException("SynopticAnalysisDomain end does not match the next calendar-month boundary",
                                          input.to_json(), Here());
        }

        if (domain.startOffsetHoursFromReference !=
            offsetHoursFromReference(anchor.referenceDateTime, realDomainStartDateTime)) {
            throw Mars2GribModelException(
                "SynopticAnalysisDomain start offset does not match resolved datetime placement", input.to_json(),
                Here());
        }

        if (domain.endOffsetHoursFromReference !=
            offsetHoursFromReference(anchor.referenceDateTime, domain.domainEndDateTime)) {
            throw Mars2GribModelException(
                "SynopticAnalysisDomain end offset does not match resolved datetime placement", input.to_json(),
                Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_SynopticAnalysis_Domain`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

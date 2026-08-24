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
/// @file AnalysisDomain.h
/// @brief Matcher, builder, and checker for the normal analysis domain.
///
/// This header is the authoritative implementation of the `AnalysisDomain`
/// domain case. It keeps recognition, construction, and validation together so
/// that the complete case can be reviewed without following a dispatch chain.
///
/// The matcher identifies the absolute-domain semantics. The builder constructs
/// all raw domain members directly from the resolved anchor and outer range.
/// The checker validates that the resolved domain remains consistent with both
/// the case semantics and the originating normalized input.
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
 * @brief Return true only when input matches the normal analysis domain.
 *
 * - the product is not synoptic;
 * - AIFS analysis is forbidden;
 * - MARS semantics classify the product as analysis.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the domain matcher fails unexpectedly.
 */
inline bool match_Analysis_Domain(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::deductions::SimulationRegime;
    using metkit::mars2grib::backend::deductions::SimulationType;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool isNotSynoptic = !input.isSynoptic;
        const bool isNotAifs     = input.regime != SimulationRegime::AIFS;
        const bool isAnalysis    = input.simulationType == SimulationType::Analysis;

        return isNotSynoptic && isNotAifs && isAnalysis;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_Analysis_Domain`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct the raw analysis domain from the resolved anchor and outer range.
 *
 * In this case:
 * - the real support start is the anchor reference datetime;
 * - the real support end is the reference datetime plus the resolved outer
 *   range;
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
inline ProductTimeSpecDomain build_Analysis_Domain(const ProductTimeSpecInput& input,
                                                   const ProductTimeSpecClassification& classification,
                                                   const anchor::ProductTimeSpecAnchor& anchor,
                                                   const shape::ProductTimeSpecOuterTimeRange& outerTimeRange) {
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecOuterTimeRangeAvailability;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::addDuration;

    try {
        (void)classification;

        const bool outerTimeRangeIsAvailable =
            outerTimeRange.availability == ProductTimeSpecOuterTimeRangeAvailability::Available;

        if (!outerTimeRangeIsAvailable || !outerTimeRange.timeRange.has_value()) {
            throw Mars2GribModelException("AnalysisDomain requires an available outer time range", input.to_json(),
                                          Here());
        }

        // In the non-synoptic analysis case, the support starts exactly at the
        // reference datetime because the analysis anchor already resolves the
        // effective reference instant.
        const auto domainStartDateTime = anchor.referenceDateTime;

        // The support end is the reference datetime extended forward by the
        // resolved outer range.
        const auto outerRange        = *outerTimeRange.timeRange;
        const auto domainEndDateTime = addDuration(anchor.referenceDateTime, outerRange);

        // This domain case is never synoptic.
        const bool isSynoptic = false;

        // The start offset is measured from the reference datetime to the real
        // support start. In this case the support starts at the reference, so
        // the offset should resolve to zero.
        const long startOffsetHoursFromReference =
            offsetHoursFromReference(anchor.referenceDateTime, domainStartDateTime);

        // The end offset is measured from the reference datetime to the support
        // end and carries the encoder-facing duration placement information.
        const long endOffsetHoursFromReference = offsetHoursFromReference(anchor.referenceDateTime, domainEndDateTime);

        return ProductTimeSpecDomain{domainStartDateTime, domainEndDateTime, isSynoptic, startOffsetHoursFromReference,
                                     endOffsetHoursFromReference};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_Analysis_Domain`", input.to_json(), Here()));
    }
}

/**
 * @brief Validate one resolved AnalysisDomain against its source input and anchor.
 *
 * This checker verifies:
 * - the domain is not synoptic;
 * - the support starts at the anchor reference datetime;
 * - the recorded hour offsets agree with the resolved start and end datetimes;
 * - the support start does not follow the support end.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @param[in] anchor Previously constructed ProductTimeSpec anchor.
 * @param[in] domain Resolved domain artifact produced by the builder.
 * @return `true` when the domain is valid for the AnalysisDomain case.
 * @throws Mars2GribModelException if the resolved domain is inconsistent with
 *         the input, anchor, or case semantics.
 */
inline bool check_Analysis_Domain(const ProductTimeSpecInput& input, const anchor::ProductTimeSpecAnchor& anchor,
                                  const ProductTimeSpecDomain& domain) {
    using metkit::mars2grib::backend::models::product_time_spec::domain::detail::offsetHoursFromReference;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        if (domain.isSynoptic) {
            throw Mars2GribModelException("AnalysisDomain must not be synoptic", input.to_json(), Here());
        }

        if (domain.domainStartDateTime != anchor.referenceDateTime) {
            throw Mars2GribModelException("AnalysisDomain start must equal anchor reference datetime", input.to_json(),
                                          Here());
        }

        if (domain.domainStartDateTime > domain.domainEndDateTime) {
            throw Mars2GribModelException("AnalysisDomain start must not follow domain end", input.to_json(), Here());
        }

        if (domain.startOffsetHoursFromReference !=
            offsetHoursFromReference(anchor.referenceDateTime, domain.domainStartDateTime)) {
            throw Mars2GribModelException("AnalysisDomain start offset does not match resolved datetime placement",
                                          input.to_json(), Here());
        }

        if (domain.endOffsetHoursFromReference !=
            offsetHoursFromReference(anchor.referenceDateTime, domain.domainEndDateTime)) {
            throw Mars2GribModelException("AnalysisDomain end offset does not match resolved datetime placement",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_Analysis_Domain`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain::detail

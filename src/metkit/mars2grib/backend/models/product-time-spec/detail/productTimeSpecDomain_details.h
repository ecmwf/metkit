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
/// @file productTimeSpecDomain_details.h
/// @brief Internal helpers for ProductTimeSpec domain construction.
///

#pragma once

#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecTimeUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Verify that the passed anchor classification matches the resolved anchor artifact.
///
/// Domain construction receives both the classification enum and the already
/// built anchor artifact. This helper checks that the two remain synchronized so
/// later placement logic never reasons over a mismatched classification/artifact
/// pair.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] anchorType Previously resolved anchor classification.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         anchor artifact and classification disagree.
///
template <class Input_t>
void checkProductTimeSpecDomainAnchorConsistency_or_throw(const Input_t& input,
                                                          TimeAnchorKind anchorType,
                                                          const ProductTimeSpecAnchor& anchor) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (anchor.anchorType != anchorType) {
        throw Mars2GribModelException(
            "ProductTimeSpec domain build received inconsistent anchor classification and anchor artifact",
            input.to_json(),
            Here());
    }
}

///
/// @brief Return the normalized resolved `step` duration for domain placement.
///
/// The domain stage needs one concrete duration for placement arithmetic. The
/// shape classifier already established whether the missing-step state is legal,
/// so this helper may treat a missing `step` as the resolved zero-duration case,
/// which today is the analysis-product compatibility branch.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return Explicit normalized `step`, or zero seconds when absent.
///
template <class Input_t>
deductions::TimeDuration buildProductTimeSpecDomainResolvedStep_or_throw(const Input_t& input) {
    if (input.step.has_value()) {
        return *input.step;
    }

    return deductions::TimeDuration{0, tables::TimeUnit::Second};
}

///
/// @brief Build the domain end datetime from the resolved anchor and normalized step.
///
/// The ProductTimeSpec domain always ends at the anchor reference datetime plus
/// the resolved forecast step. The helper centralizes that arithmetic so all
/// shape branches share the same notion of the support end.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @return Absolute `domainEndDateTime` for the product support.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         normalized step cannot be applied to the reference datetime.
///
template <class Input_t>
eckit::DateTime buildProductTimeSpecDomainEndDateTime_or_throw(const Input_t& input,
                                                               const ProductTimeSpecAnchor& anchor) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;


    try {
        return addProductTimeSpecDuration_or_throw(
            anchor.referenceDateTime,
            buildProductTimeSpecDomainResolvedStep_or_throw(input));
    } catch (const Mars2GribModelException&) {
        throw;
    } catch (const std::exception& e) {
        throw Mars2GribModelException(
            std::string("Failed to compute ProductTimeSpec domain end datetime: ") + e.what(),
            input.to_json(),
            Here());
    }
}

///
/// @brief Select the outermost range used to place the absolute product support.
///
/// Domain placement depends only on the outermost temporal support range for the
/// active shape classification.
///
/// The current mapping is:
/// - `Instant` -> zero-second synthetic range;
/// - `StandardSingleLoop` -> normalized `timespan.duration`;
/// - `FakeSingleLoopDoubleLoop` -> normalized `timespan.duration`;
/// - `MultiLoop` -> first parsed `stattype` block range;
/// - `FakeDoubleLoopSingleLoop` -> first parsed `stattype` block range;
/// - `FromStartSingleLoop` -> resolved `step` duration.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @return The outermost normalized range used for support placement.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         required source range is absent for the chosen shape.
///
template <class Input_t>
deductions::TimeDuration buildProductTimeSpecDomainOutermostRange_or_throw(const Input_t& input,
                                                                           ProductTimeSpecShapeKind shapeType) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    switch (shapeType) {
        case ProductTimeSpecShapeKind::Instant:
            return deductions::TimeDuration{0, tables::TimeUnit::Second};

        case ProductTimeSpecShapeKind::StandardSingleLoop:
        case ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop:
            if (!input.timespan.has_value() || !input.timespan->duration.has_value()) {
                throw Mars2GribModelException(
                    "Duration-valued ProductTimeSpec shape is missing normalized `timespan.duration`",
                    input.to_json(),
                    Here());
            }
            return *input.timespan->duration;

        case ProductTimeSpecShapeKind::MultiLoop:
        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
            if (!input.stattype.has_value() || input.stattype->empty()) {
                throw Mars2GribModelException(
                    "ProductTimeSpec shape requires at least one parsed `stattype` block for domain placement",
                    input.to_json(),
                    Here());
            }
            return (*input.stattype)[0].timeRange;

        case ProductTimeSpecShapeKind::FromStartSingleLoop:
            return buildProductTimeSpecDomainResolvedStep_or_throw(input);
    }

    throw Mars2GribModelException(
        "Unhandled ProductTimeSpec shape classification while selecting the domain outermost range",
        input.to_json(),
        Here());
}

///
/// @brief Enforce strict calendar alignment for day- and month-based outer ranges.
///
/// Day-based outer ranges require the support end at midnight. Month-based
/// outer ranges require the support end on the first day of the month at
/// midnight. Elapsed second- and hour-based ranges impose no additional
/// calendar alignment requirement here.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] domainEndDateTime Candidate support end datetime.
/// @param[in] outermostRange Selected outermost normalized range.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         calendar alignment rule is violated.
///
template <class Input_t>
void checkProductTimeSpecDomainOutermostAlignment_or_throw(const Input_t& input,
                                                           const eckit::DateTime& domainEndDateTime,
                                                           const deductions::TimeDuration& outermostRange) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (outermostRange.unit == tables::TimeUnit::Day && !isAtMidnight(domainEndDateTime)) {
        throw Mars2GribModelException(
            "Day-based ProductTimeSpec outermost range requires `domainEndDateTime` at midnight",
            input.to_json(),
            Here());
    }
    if (outermostRange.unit == tables::TimeUnit::Month &&
        !isOnFirstOfMonthMidnight(domainEndDateTime)) {
        throw Mars2GribModelException(
            "Month-based ProductTimeSpec outermost range requires `domainEndDateTime` on day 1 at midnight",
            input.to_json(),
            Here());
    }
}

///
/// @brief Build the domain start datetime from the active shape placement rule.
///
/// The start-datetime rule depends on the shape classification:
/// - `Instant` -> `domainEndDateTime`;
/// - `FromStartSingleLoop` -> `anchor.referenceDateTime`;
/// - every other currently supported shape -> `domainEndDateTime - outermostRange`.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @param[in] domainEndDateTime Absolute support end datetime.
/// @param[in] outermostRange Selected outermost normalized range.
/// @return Absolute `domainStartDateTime` for the product support.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         start datetime cannot be placed for the selected shape.
///
template <class Input_t>
eckit::DateTime buildProductTimeSpecDomainStartDateTime_or_throw(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType,
    const ProductTimeSpecAnchor& anchor,
    const eckit::DateTime& domainEndDateTime,
    const deductions::TimeDuration& outermostRange) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        switch (shapeType) {
            case ProductTimeSpecShapeKind::Instant:
                return domainEndDateTime;

            case ProductTimeSpecShapeKind::FromStartSingleLoop:
                return anchor.referenceDateTime;

            case ProductTimeSpecShapeKind::StandardSingleLoop:
            case ProductTimeSpecShapeKind::MultiLoop:
            case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
            case ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop:
                return subtractProductTimeSpecDuration_or_throw(domainEndDateTime, outermostRange);
        }
    } catch (const Mars2GribModelException&) {
        throw;
    } catch (const std::exception& e) {
        throw Mars2GribModelException(
            std::string("Failed to compute ProductTimeSpec domain start datetime: ") + e.what(),
            input.to_json(),
            Here());
    }

    throw Mars2GribModelException(
        "Unhandled ProductTimeSpec shape classification while building the domain start datetime",
        input.to_json(),
        Here());
}

///
/// @brief Validate whole-domain placement consistency after start/end materialization.
///
/// The domain builder validates the following placement invariants:
/// - `domainStartDateTime <= domainEndDateTime` always;
/// - `FromStartSingleLoop` starts exactly at `anchor.referenceDateTime`;
/// - every non-instant support starts at or after `anchor.referenceDateTime`;
/// - for from-start placement, subtracting the resolved outermost range from the
///   support end must reproduce the reference datetime.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] shapeType Previously resolved valid shape classification.
/// @param[in] anchor Already built ProductTimeSpec anchor artifact.
/// @param[in] domainStartDateTime Absolute support start datetime.
/// @param[in] domainEndDateTime Absolute support end datetime.
/// @param[in] outermostRange Selected outermost normalized range.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException when a
///         placement invariant is violated.
///
template <class Input_t>
void checkProductTimeSpecDomainConsistency_or_throw(const Input_t& input,
                                                    ProductTimeSpecShapeKind shapeType,
                                                    const ProductTimeSpecAnchor& anchor,
                                                    const eckit::DateTime& domainStartDateTime,
                                                    const eckit::DateTime& domainEndDateTime,
                                                    const deductions::TimeDuration& outermostRange) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (domainStartDateTime > domainEndDateTime) {
        throw Mars2GribModelException(
            "ProductTimeSpec domain placement produced `domainStartDateTime > domainEndDateTime`",
            input.to_json(),
            Here());
    }

    if (shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop) {
        if (domainStartDateTime != anchor.referenceDateTime) {
            throw Mars2GribModelException(
                "From-start ProductTimeSpec domain must begin exactly at `referenceDateTime`",
                input.to_json(),
                Here());
        }

        try {
            if (subtractProductTimeSpecDuration_or_throw(domainEndDateTime, outermostRange) !=
                anchor.referenceDateTime) {
                throw Mars2GribModelException(
                    "From-start ProductTimeSpec domain is inconsistent with the resolved support end and outermost range",
                    input.to_json(),
                    Here());
            }
        } catch (const Mars2GribModelException&) {
            throw;
        } catch (const std::exception& e) {
            throw Mars2GribModelException(
                std::string("Failed to validate from-start ProductTimeSpec domain consistency: ") + e.what(),
                input.to_json(),
                Here());
        }
    }

    if (shapeType != ProductTimeSpecShapeKind::Instant &&
        domainStartDateTime < anchor.referenceDateTime) {
        throw Mars2GribModelException(
            "ProductTimeSpec statistical support begins before `referenceDateTime`",
            input.to_json(),
            Here());
    }
}

}  // namespace metkit::mars2grib::backend::models::detail

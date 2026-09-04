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
/// @file ForecastAnalysis.h
/// @brief Matcher, builder, and checker for the ForecastAnalysis anchor case.
///
/// This header is the authoritative implementation of the `ForecastAnalysis`
/// anchor case. It keeps recognition and construction together so that the
/// complete case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and returns
/// their explicit conjunction. The builder constructs the three canonical anchor
/// members directly from the visible MARS `date` and optional MARS `time`
/// source. The checker validates that the resolved anchor remains consistent
/// with both the classification and the originating input fields.
///
/// All functions are deterministic, have no externally visible side effects,
/// catch every failure, and rethrow a nested `Mars2GribModelException` carrying
/// the normalized input snapshot.
///
/// @ingroup mars2grib_product_time_spec_anchors
///
#pragma once

#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail {

/**
 * @brief Return true only when input matches the ForecastAnalysis anchor case.
 *
 * - MARS date is present;
 * - MARS time is optional;
 * - hdate is absent;
 * - year and month are absent.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the anchor matcher fails unexpectedly.
 */
inline bool match_ForecastAnalysis_Anchor(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasDate    = input.marsDate.has_value();
        const bool hasNoHdate = !input.marsHdate.has_value();
        const bool hasNoYear  = !input.marsYear.has_value();
        const bool hasNoMonth = !input.marsMonth.has_value();

        return hasDate && hasNoHdate && hasNoYear && hasNoMonth;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_ForecastAnalysis_Anchor`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct a ForecastAnalysis anchor from one MARS date/time source.
 *
 * This case has one direct source datetime: MARS `date` plus optional MARS
 * `time`. The three canonical anchor members are therefore identical.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed and order-validated ProductTimeSpec anchor.
 *
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecAnchor build_ForecastAnalysis_Anchor(const ProductTimeSpecInput& input,
                                                           const ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchorKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {

        // This case requires a MARS date and optionally a MARS time.
        if (!input.marsDate.has_value()) {
            throw Mars2GribModelException("ForecastAnalysis anchor construction requires MARS date", input.to_json(),
                                          Here());
        }

        // The three canonical anchor members are identical in this case because
        // the product exposes only one direct source datetime.
        // `makeDateTime(...)` injects the canonical `00:00:00` default when the
        // optional MARS time is absent.
        const eckit::DateTime labelDateTime             = makeDateTime(input.marsDate.value(), input.marsTime);
        const eckit::DateTime initialConditionsDateTime = makeDateTime(input.marsDate.value(), input.marsTime);
        const eckit::DateTime referenceDateTime         = makeDateTime(input.marsDate.value(), input.marsTime);

        // Validate the canonical ordering invariant and return the final anchor.
        return checkedAnchor(labelDateTime, initialConditionsDateTime, referenceDateTime,
                             ProductTimeSpecAnchorKind::ForecastAnalysis);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_ForecastAnalysis_Anchor`", input.to_json(), Here()));
    }
}

/// @brief Validate one resolved ForecastAnalysis anchor against its source input.
///
/// This checker verifies:
/// - the resolved anchor type;
/// - equality of the three canonical anchor datetimes for this case;
/// - agreement with the originating MARS `date` and optional MARS `time`.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @param[in] anchor Resolved anchor artifact produced by the builder.
/// @return `true` when the anchor is valid for the ForecastAnalysis case.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if
///         the resolved anchor is inconsistent with the input or case semantics.
inline bool check_ForecastAnalysis_Anchor(const ProductTimeSpecInput& input, const ProductTimeSpecAnchor& anchor) {
    using metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchorKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        if (anchor.anchorType != ProductTimeSpecAnchorKind::ForecastAnalysis) {
            throw Mars2GribModelException("Anchor type mismatch: expected ForecastAnalysis", input.to_json(), Here());
        }

        if (anchor.labelDateTime != anchor.initialConditionsDateTime ||
            anchor.initialConditionsDateTime != anchor.referenceDateTime) {
            throw Mars2GribModelException("Anchor datetimes are not equal for ForecastAnalysis", input.to_json(),
                                          Here());
        }

        if (!input.marsDate.has_value()) {
            throw Mars2GribModelException("Input missing MARS date for ForecastAnalysis", input.to_json(), Here());
        }

        if (input.marsDate.value() != anchor.labelDateTime.date()) {
            throw Mars2GribModelException("Anchor label date does not match input MARS date", input.to_json(), Here());
        }

        if (input.marsTime.value_or(eckit::Time{0, 0, 0}) != anchor.labelDateTime.time()) {
            throw Mars2GribModelException("Anchor label time does not match input MARS time", input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_ForecastAnalysis_Anchor`", input.to_json(), Here()));
    }
}


}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail

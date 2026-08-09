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
/// @file Hindcast.h
/// @brief Matcher, builder, and checker for the Hindcast anchor case.
///
/// This header is the authoritative implementation of the `Hindcast` anchor
/// case. It keeps recognition and construction together so that the complete
/// case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and returns
/// their explicit conjunction. The builder constructs the three canonical anchor
/// members directly from the visible MARS `hdate`, MARS `date`, and optional
/// MARS `time` source fields. The checker validates that the resolved anchor
/// remains consistent with both the classification and the originating input.
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
 * @brief Return true only when input matches the Hindcast anchor case.
 *
 * - MARS date is present;
 * - MARS time is optional;
 * - hdate is present;
 * - year and month are absent.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the anchor matcher fails unexpectedly.
 */
inline bool match_Hindcast_Anchor(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasDate    = input.marsDate.has_value();
        const bool hasHdate   = input.marsHdate.has_value();
        const bool hasNoYear  = !input.marsYear.has_value();
        const bool hasNoMonth = !input.marsMonth.has_value();

        return hasDate && hasHdate && hasNoYear && hasNoMonth;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_Hindcast_Anchor`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct a Hindcast anchor from hdate and MARS date/time.
 *
 * In this case:
 * - `labelDateTime` is the hindcast date at `00:00:00`;
 * - `initialConditionsDateTime` is built from MARS `date` and optional MARS
 *   `time`;
 * - `referenceDateTime` equals `initialConditionsDateTime`.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Constructed and order-validated ProductTimeSpec anchor.
 *
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecAnchor build_Hindcast_Anchor(const ProductTimeSpecInput& input,
                                                   const ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchorKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        (void)classification;

        if (!input.marsHdate.has_value()) {
            throw Mars2GribModelException("Hindcast anchor construction requires hdate", input.to_json(), Here());
        }

        if (!input.marsDate.has_value()) {
            throw Mars2GribModelException("Hindcast anchor construction requires MARS date", input.to_json(), Here());
        }

        // In the Hindcast case, the label is the hindcast date at midnight,
        // while the initial conditions and reference are derived from the MARS
        // date plus optional MARS time.
        const eckit::DateTime labelDateTime             = makeDateTime(*input.marsHdate, defaultMarsTime());
        const eckit::DateTime initialConditionsDateTime = makeDateTime(*input.marsDate, input.marsTime);
        const eckit::DateTime referenceDateTime         = initialConditionsDateTime;

        return checkedAnchor(labelDateTime, initialConditionsDateTime, referenceDateTime,
                             ProductTimeSpecAnchorKind::Hindcast);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_Hindcast_Anchor`", input.to_json(), Here()));
    }
}

/// @brief Validate one resolved Hindcast anchor against its source input.
///
/// This checker verifies:
/// - the resolved anchor type;
/// - the expected equality between initial-conditions and reference datetimes;
/// - agreement with the input `hdate`, MARS `date`, and optional MARS `time`.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @param[in] anchor Resolved anchor artifact produced by the builder.
/// @return `true` when the anchor is valid for the Hindcast case.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if
///         the resolved anchor is inconsistent with the input or case semantics.
inline bool check_Hindcast_Anchor(const ProductTimeSpecInput& input, const ProductTimeSpecAnchor& anchor) {
    using metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchorKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;

    try {

        if (anchor.anchorType != ProductTimeSpecAnchorKind::Hindcast) {
            throw Mars2GribModelException("Anchor type mismatch: expected Hindcast", input.to_json(), Here());
        }

        if (anchor.initialConditionsDateTime != anchor.referenceDateTime) {
            throw Mars2GribModelException(
                "Initial conditions datetime and reference datetime must be equal for Hindcast", input.to_json(),
                Here());
        }

        if (anchor.labelDateTime > anchor.initialConditionsDateTime) {
            throw Mars2GribModelException("Label datetime must not follow initial conditions datetime for Hindcast",
                                          input.to_json(), Here());
        }

        if (!input.marsHdate.has_value()) {
            throw Mars2GribModelException("Input missing hdate for Hindcast", input.to_json(), Here());
        }

        if (!input.marsDate.has_value()) {
            throw Mars2GribModelException("Input missing MARS date for Hindcast", input.to_json(), Here());
        }

        if (anchor.labelDateTime.date() != *input.marsHdate) {
            throw Mars2GribModelException("Anchor label date does not match input hdate", input.to_json(), Here());
        }

        if (anchor.labelDateTime.time() != defaultMarsTime()) {
            throw Mars2GribModelException("Anchor label time must be 00:00:00 for Hindcast", input.to_json(), Here());
        }

        if (anchor.initialConditionsDateTime.date() != *input.marsDate) {
            throw Mars2GribModelException("Anchor initial conditions date does not match input MARS date",
                                          input.to_json(), Here());
        }

        if (anchor.initialConditionsDateTime.time() != input.marsTime.value_or(defaultMarsTime())) {
            throw Mars2GribModelException("Anchor initial conditions time does not match input MARS time",
                                          input.to_json(), Here());
        }

        return true;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_Hindcast_Anchor`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail

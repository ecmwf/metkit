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
/// @file SeasonalClimate.h
/// @brief Matcher and builder for the SeasonalClimate anchor case.
///
/// This header is the authoritative implementation of the `SeasonalClimate`
/// anchor case. It keeps recognition and construction together so that the
/// complete case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and returns
/// their explicit conjunction. The builder constructs the three canonical anchor
/// times for the same case and delegates only shared date/time primitives and
/// ordering validation.
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

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail {

/**
 * @brief Return true only when input matches the SeasonalClimate anchor case.
 *
 * - MARS date is absent;
 * - MARS time is absent;
 * - hdate is absent;
 * - year and month are present.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `true` only when all documented conditions are satisfied; otherwise `false`.
 * @throws Mars2GribModelException If evaluating the anchor matcher fails unexpectedly.
 */
inline bool match_SeasonalClimate_Anchor(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasNoDate             = !input.marsDate.has_value();
        const bool hasNoTime             = !input.marsTime.has_value();
        const bool hasNoHdate            = !input.marsHdate.has_value();
        const bool hasYearMonth          = hasCompleteYearMonth(input);
        const bool hasNoPartialYearMonth = !hasPartialYearMonth(input);

        return hasNoDate && hasNoTime && hasNoHdate && hasYearMonth && hasNoPartialYearMonth;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalClimate_Anchor`", input.to_json(), Here()));
    }
}

/**
 * @brief Construct a SeasonalClimate anchor from MARS year and month.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @return Constructed and order-validated ProductTimeSpec anchor.
 *
 * @throws Mars2GribModelException If construction detects an invalid or inconsistent state.
 */
inline ProductTimeSpecAnchor build_SeasonalClimate_Anchor(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::backend::models::product_time_spec::anchor::ProductTimeSpecAnchorKind;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const eckit::DateTime anchorDateTime = seasonalClimateDateTimeFromInput(input);

        return checkedAnchor(input, anchorDateTime, anchorDateTime, anchorDateTime,
                             ProductTimeSpecAnchorKind::SeasonalClimate);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_SeasonalClimate_Anchor`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail

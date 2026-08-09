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
/// @brief Matcher, builder, and checker for the SeasonalClimate anchor case.
///
/// This header is the authoritative implementation of the `SeasonalClimate`
/// anchor case. It keeps recognition and construction together so that the
/// complete case can be reviewed without following a dispatch chain.
///
/// The matcher states every identifying condition as a named Boolean and returns
/// their explicit conjunction. This case is currently reserved but not
/// implemented: the matcher, builder, and checker all raise explicit
/// `not implemented` model exceptions when the seasonal-climate source pattern
/// is encountered.
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
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail {

/**
 * @brief Identify the reserved SeasonalClimate anchor source pattern.
 *
 * This implementation currently does not support SeasonalClimate anchors.
 * If either `marsYear` or `marsMonth` is present, the recognized seasonal
 * anchor pattern is treated as explicitly not implemented.
 *
 * @param[in] input Fully normalized ProductTimeSpec input snapshot.
 * @return `false` when no seasonal-climate source fields are present.
 * @throws Mars2GribModelException if the seasonal-climate source pattern is
 *         encountered, because the case is intentionally not implemented.
 */
inline bool match_SeasonalClimate_Anchor(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasYear  = input.marsYear.has_value();
        const bool hasMonth = input.marsMonth.has_value();

        if (hasYear || hasMonth) {
            throw Mars2GribModelException("SeasonalClimate anchor is not implemented", input.to_json(), Here());
        }

        return false;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `match_SeasonalClimate_Anchor`", input.to_json(), Here()));
    }
}

/**
 * @brief Refuse construction of the reserved SeasonalClimate anchor case.
 *
 * This case is intentionally not implemented at the moment.
 *
 * @param[in] input Fully normalized ProductTimeSpec input.
 * @param[in] classification Full resolved ProductTimeSpec classification bundle.
 * @return Never returns successfully.
 *
 * @throws Mars2GribModelException always, because SeasonalClimate anchor
 *         construction is intentionally not implemented.
 */
inline ProductTimeSpecAnchor build_SeasonalClimate_Anchor(const ProductTimeSpecInput& input,
                                                          const ProductTimeSpecClassification& classification) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)classification;

        throw Mars2GribModelException("SeasonalClimate anchor is not implemented", input.to_json(), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `build_SeasonalClimate_Anchor`", input.to_json(), Here()));
    }
}

/// @brief Refuse validation of the reserved SeasonalClimate anchor case.
///
/// This checker is present so the anchor registry has a complete checker table,
/// but the SeasonalClimate case is intentionally not implemented.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @param[in] anchor Resolved anchor artifact placeholder.
/// @return Never returns successfully.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException
///         always, because SeasonalClimate anchor validation is intentionally
///         not implemented.
inline bool check_SeasonalClimate_Anchor(const ProductTimeSpecInput& input, const ProductTimeSpecAnchor& anchor) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        (void)anchor;

        throw Mars2GribModelException("SeasonalClimate anchor is not implemented", input.to_json(), Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to execute `check_SeasonalClimate_Anchor`", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail

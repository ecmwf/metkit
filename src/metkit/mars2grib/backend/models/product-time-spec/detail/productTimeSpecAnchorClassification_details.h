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
/// @file productTimeSpecAnchorClassification_details.h
/// @brief Internal helpers for ProductTimeSpec anchor classification.
///

#pragma once

#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Validate that at least one direct anchor source is available.
///
/// Anchor classification requires at least one direct source among `dateTime`,
/// `hindcastDateTime`, and `yearMonthDateTime`. The resolved label anchor may be
/// supplied by either `hindcastDateTime`, `dateTime`, or `yearMonthDateTime`, so
/// the complete absence of all three is the only locally invalid direct-source
/// state for classification.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if all
///         direct anchor sources are absent.
///
template <class Input_t>
void checkDirectAnchorSourceAvailability_or_throw(const Input_t& input) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (!input.dateTime.has_value() && !input.hindcastDateTime.has_value() &&
        !input.yearMonthDateTime.has_value()) {
        throw Mars2GribModelException(
            "No direct ProductTimeSpec anchor source is present in normalized input",
            input.to_json(),
            Here());
    }
}

}  // namespace metkit::mars2grib::backend::models::detail

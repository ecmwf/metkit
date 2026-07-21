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
/// @file ProductTimeSpecAnchorClassification.h
/// @brief Public anchor classification surface for ProductTimeSpec.
///
/// Exposes the anchor-classification public model API:
/// - `TimeAnchorKind`, the anchor classification enum;
/// - `classify_ProductTimeSpecAnchor_or_throw(...)`.
///
/// This header owns only the public anchor-classification types, the public
/// classification entry point, and the normative documentation of the anchor
/// classification model. Internal helper logic lives in
/// `detail/productTimeSpecAnchorClassification_details.h`.
///
/// Anchor classification determines the direct special-anchor source regime used
/// later by anchor building. Classification depends only on the presence of the
/// two special direct anchor sources carried by the normalized model input. The
/// table below also lists `dateTime` explicitly because it participates in
/// anchor materialization even though it does not create an additional
/// classification case:
///
/// | `dateTime` | `hindcastDateTime` | `yearMonthDateTime` | `TimeAnchorKind`         | Notes                                  |
/// |------------|--------------------|---------------------|--------------------------|----------------------------------------|
/// | present    | absent             | absent              | `LabelOnly`              | `dateTime` is the direct initial-conditions source |
/// | present    | present            | absent              | `Hindcast`               | `hindcastDateTime` is the direct label source |
/// | present    | absent             | present             | `ForecastAnchor`         | `yearMonthDateTime` drives the reference source |
/// | present    | present            | present             | `HindcastForecastAnchor` | both special direct anchor sources present |
/// | absent     | present            | absent              | `Hindcast`               | `hindcastDateTime` is the direct label source |
/// | absent     | absent             | present             | `ForecastAnchor`         | label falls back to `yearMonthDateTime` |
/// | absent     | present            | present             | `HindcastForecastAnchor` | `hindcastDateTime` is the direct label source |
/// | absent     | absent             | absent              | invalid                  | no direct anchor source                |
///
/// `dateTime` participates in anchor materialization but does not introduce a
/// separate classification case. It is used as the direct
/// initial-conditions source when present. Classification rejects only the
/// unsupported case where every direct anchor source is absent.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <cstddef>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecAnchorClassification_details.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {

///
/// @brief Direct special-anchor source regime used to build the ordered anchor.
///
/// The classification depends only on the presence of the normalized direct
/// special-anchor sources `hindcastDateTime` and `yearMonthDateTime`.
/// `dateTime` participates only in anchor materialization.
///
enum class TimeAnchorKind : std::size_t {
    LabelOnly,
    Hindcast,
    ForecastAnchor,
    HindcastForecastAnchor
};

///
/// @brief Classify the anchor source regime from normalized model input.
///
/// The classification uses only the presence of `hindcastDateTime` and
/// `yearMonthDateTime`. It rejects the single unsupported direct-source state in
/// which no direct anchor source is available at all: `dateTime`,
/// `hindcastDateTime`, and `yearMonthDateTime` all absent.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return The resolved `TimeAnchorKind` classification.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         classification failure, with `input.to_json()` attached as context.
///
template <class Input_t>
TimeAnchorKind classify_ProductTimeSpecAnchor_or_throw(const Input_t& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        detail::checkDirectAnchorSourceAvailability_or_throw(input);

        const bool hasHindcastDateTime = input.hindcastDateTime.has_value();
        const bool hasYearMonthDateTime = input.yearMonthDateTime.has_value();

        if (!hasHindcastDateTime && !hasYearMonthDateTime) {
            return TimeAnchorKind::LabelOnly;
        }
        if (hasHindcastDateTime && !hasYearMonthDateTime) {
            return TimeAnchorKind::Hindcast;
        }
        if (!hasHindcastDateTime && hasYearMonthDateTime) {
            return TimeAnchorKind::ForecastAnchor;
        }
        return TimeAnchorKind::HindcastForecastAnchor;
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to classify `ProductTimeSpecAnchor` from normalized input",
            input.to_json(),
            Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models

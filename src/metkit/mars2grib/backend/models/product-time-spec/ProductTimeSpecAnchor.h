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
/// @file ProductTimeSpecAnchor.h
/// @brief Public anchor build surface for ProductTimeSpec.
///
/// Exposes the anchor-build public model API:
/// - `ProductTimeSpecAnchor`, the resolved anchor artifact;
/// - `build_ProductTimeSpecAnchor_or_throw(...)`.
///
/// This header intentionally stays small. It owns only the public anchor build
/// artifact, the public build entry point, and the normative documentation of
/// anchor materialization. Anchor classification is documented separately in
/// `ProductTimeSpecAnchorClassification.h`. Internal helper logic lives in
/// `detail/productTimeSpecAnchor_details.h`.
///
/// The resolved anchor datetimes are built with the following precedence and
/// inheritance rules:
/// - `labelDateTime`: `hindcastDateTime`, otherwise `dateTime`, otherwise
///   `yearMonthDateTime`;
/// - `initialConditionsDateTime`: `dateTime` when present, otherwise
///   `labelDateTime`;
/// - `referenceDateTime`: `yearMonthDateTime` when present, otherwise
///   `initialConditionsDateTime`.
///
/// The resulting truth table is:
///
/// | `dateTime` | `hindcastDateTime` | `yearMonthDateTime` | `labelDateTime` | `initialConditionsDateTime` | `referenceDateTime` |
/// |------------|--------------------|---------------------|-----------------|-----------------------------|---------------------|
/// | present    | absent             | absent              | `dateTime`      | `dateTime`                  | `dateTime`          |
/// | present    | present            | absent              | `hindcastDateTime` | `dateTime`               | `dateTime`          |
/// | present    | absent             | present             | `dateTime`      | `dateTime`                  | `yearMonthDateTime` |
/// | present    | present            | present             | `hindcastDateTime` | `dateTime`               | `yearMonthDateTime` |
/// | absent     | present            | absent              | `hindcastDateTime` | `hindcastDateTime`       | `hindcastDateTime`  |
/// | absent     | absent             | present             | `yearMonthDateTime` | `yearMonthDateTime`     | `yearMonthDateTime` |
/// | absent     | present            | present             | `hindcastDateTime` | `hindcastDateTime`       | `yearMonthDateTime` |
/// | absent     | absent             | absent              | invalid         | invalid                     | invalid             |
///
/// Every successfully built anchor must satisfy the invariant:
/// `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <cstddef>

#include "eckit/types/DateTime.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchorClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecAnchor_details.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDataTypes.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {


///
/// @brief Build the fully resolved anchor artifact from input and classification.
///
/// The build stage materializes `labelDateTime`,
/// `initialConditionsDateTime`, and `referenceDateTime` using the documented
/// precedence and inheritance rules, then validates the required ordering
/// invariant.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] anchorType Previously resolved valid anchor classification.
/// @return Fully resolved `ProductTimeSpecAnchor` artifact.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         build or validation failure, with `input.to_json()` attached as
///         context.
///
template <class Input_t>
ProductTimeSpecAnchor build_ProductTimeSpecAnchor_or_throw(const Input_t& input,
                                                           TimeAnchorKind anchorType) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const eckit::DateTime labelDateTime = detail::buildDirectLabelDateTime_or_throw(input);
        const eckit::DateTime initialConditionsDateTime =
            detail::buildDirectInitialConditionsDateTime_or_throw(input, labelDateTime);
        const eckit::DateTime referenceDateTime =
            detail::buildDirectReferenceDateTime_or_throw(input, initialConditionsDateTime);

        detail::checkProductTimeSpecAnchorOrdering_or_throw(
            labelDateTime,
            initialConditionsDateTime,
            referenceDateTime,
            input);

        return ProductTimeSpecAnchor{labelDateTime,
                                     initialConditionsDateTime,
                                     referenceDateTime,
                                     anchorType};
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to build `ProductTimeSpecAnchor` from normalized input",
            input.to_json(),
            Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models

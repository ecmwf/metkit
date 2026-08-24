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
/// @file AnchorDataTypes.h
/// @brief Anchor artifact and low-level anchor invariant helpers.
///
/// This header owns:
/// - the resolved `ProductTimeSpecAnchor` artifact type;
/// - the low-level `checkedAnchor(...)` helper that validates anchor ordering
///   without depending on model-input debug context;
/// - the best-effort diagnostic JSON serializer for the resolved anchor.
///

#pragma once

#include <sstream>
#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor {

///
/// @brief Fully resolved ordered ProductTimeSpec anchor artifact.
///
/// The artifact stores the three anchor datetimes used by later ProductTimeSpec
/// stages together with the direct-source regime that produced them. The three
/// values are already fully inherited/defaulted by the selected anchor builder.
///
/// Valid resolved anchors satisfy:
/// `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
///
struct ProductTimeSpecAnchor {
    /// @brief Final label datetime used by downstream ProductTimeSpec logic.
    eckit::DateTime labelDateTime{};

    /// @brief Final initial-conditions datetime used by downstream ProductTimeSpec logic.
    eckit::DateTime initialConditionsDateTime{};

    /// @brief Final reference datetime used by downstream ProductTimeSpec logic.
    eckit::DateTime referenceDateTime{};

    /// @brief Anchor case that produced this resolved artifact.
    ProductTimeSpecAnchorKind anchorType{ProductTimeSpecAnchorKind::ForecastAnalysis};
};

/// @brief Validate anchor ordering and construct a ProductTimeSpec anchor.
///
/// This helper is intentionally input-free. It owns only the low-level anchor
/// invariant:
/// `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
///
/// Higher-level callbacks remain responsible for catching failures here and
/// rethrowing with richer `ProductTimeSpecInput` context.
/// @param[in] label Candidate label datetime.
/// @param[in] initialConditions Candidate initial-conditions datetime.
/// @param[in] reference Candidate reference datetime.
/// @param[in] anchorType Resolved anchor classification.
/// @return Complete and order-validated ProductTimeSpec anchor.
/// @throws Mars2GribGenericException If ordering is invalid or construction fails.
inline ProductTimeSpecAnchor checkedAnchor(const eckit::DateTime& label, const eckit::DateTime& initialConditions,
                                           const eckit::DateTime& reference, ProductTimeSpecAnchorKind anchorType) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        const bool labelDoesNotFollowInitialConditions   = label <= initialConditions;
        const bool initialConditionsDoNotFollowReference = initialConditions <= reference;
        const bool orderingIsValid = labelDoesNotFollowInitialConditions && initialConditionsDoNotFollowReference;

        if (!orderingIsValid) {
            throw Mars2GribGenericException("Anchor ordering must satisfy label <= initial conditions <= reference",
                                            Here());
        }

        return ProductTimeSpecAnchor{label, initialConditions, reference, anchorType};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Failed to validate and construct ProductTimeSpec anchor", Here()));
    }
}

/// @brief Serialize one resolved anchor artifact as diagnostic JSON.
///
/// This function is best-effort and never throws. It is intended for
/// diagnostic-context construction only and therefore returns a stable fallback
/// error object if serialization fails.
///
/// @param[in] value Resolved anchor artifact.
/// @return One JSON object describing the final anchor state, or a stable
///         fallback error object if serialization fails.
inline std::string productTimeSpecAnchorJson(const ProductTimeSpecAnchor& value) noexcept {
    try {
        std::ostringstream out;
        out << '{' << detail::jsonQuote_modelInput("labelDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.labelDateTime) << ','
            << detail::jsonQuote_modelInput("initialConditionsDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.initialConditionsDateTime) << ','
            << detail::jsonQuote_modelInput("referenceDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.referenceDateTime) << ','
            << detail::jsonQuote_modelInput("anchorType") << ':'
            << detail::jsonQuote_modelInput(productTimeSpecAnchorTypeName(value.anchorType)) << '}';
        return out.str();
    }
    catch (...) {
        return std::string{"{\"error\":\"productTimeSpecAnchorJson failed while building diagnostic context\"}"};
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor

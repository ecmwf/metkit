#pragma once

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

/// @brief Serialize one resolved anchor artifact as diagnostic JSON.
/// @param[in] value Resolved anchor artifact.
/// @return One JSON object describing the final anchor state.
inline std::string productTimeSpecAnchorJson(const ProductTimeSpecAnchor& value) {
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

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor

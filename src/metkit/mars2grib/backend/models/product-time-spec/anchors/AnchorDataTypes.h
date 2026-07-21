#pragma once

#include <cstddef>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor {

///
/// @brief Supported temporal-anchor construction cases.
///
/// The active backend-model anchor cases are:
/// - `ForecastAnalysis`, for products carrying one direct MARS `date` source and
///   an optional MARS `time` source;
/// - `Hindcast`, for products carrying both direct MARS `date` and direct MARS
///   `hdate` sources;
/// - `SeasonalClimate`, for products carrying MARS `year` and `month` without
///   direct `date`, `time`, or `hdate` sources.
///
/// Each case represents one unique source regime that is matched exhaustively by
/// `AnchorRegistry.h` and then materialized by one dedicated case builder.
///
/// `Count` is a sentinel used exclusively to size the registry.
///
enum class ProductTimeSpecAnchorKind : std::size_t {
    ForecastAnalysis,
    Hindcast,
    SeasonalClimate,
    Count  // Used for sizing the registry, not a valid classification.
};

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

/// @brief Return the stable diagnostic name of one anchor case.
/// @param[in] value Anchor classification value.
/// @return Stable human-readable anchor-case name.
inline std::string productTimeSpecAnchorTypeName(ProductTimeSpecAnchorKind value) {
    switch (value) {
        case ProductTimeSpecAnchorKind::ForecastAnalysis:
            return "ForecastAnalysis";
        case ProductTimeSpecAnchorKind::Hindcast:
            return "Hindcast";
        case ProductTimeSpecAnchorKind::SeasonalClimate:
            return "SeasonalClimate";
    }

    return "InvalidTimeAnchorKind";
}

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

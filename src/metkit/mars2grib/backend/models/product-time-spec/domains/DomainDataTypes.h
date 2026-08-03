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
/// @file DomainDataTypes.h
/// @brief Domain classifications and resolved ProductTimeSpec domain artifact.
///

#pragma once

#include <cstddef>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain {

///
/// @brief Supported absolute-domain construction strategies.
///
/// The active backend-model domain cases are:
/// - `ForecastDomain`, for non-synoptic non-seasonal forecast products whose
///   support ends at `referenceDateTime + step` and extends backward by the
///   resolved outer range;
/// - `SeasonalForecastDomain`, for non-synoptic seasonal forecast products whose
///   support ends at `referenceDateTime + fcmonth months` and extends backward
///   by the resolved outer range;
/// - `AnalysisDomain`, for non-synoptic analysis products whose support starts
///   at `referenceDateTime` and extends forward by the resolved outer range;
/// - `SynopticAnalysisDomain`, for synoptic IFS analysis products whose support
///   starts at the exact MARS date/time and ends at the beginning of the next
///   calendar month.
///
/// `Count` is a sentinel used exclusively to size the registry.
///
enum class ProductTimeSpecDomainKind : std::size_t {
    ForecastDomain,
    SeasonalForecastDomain,
    AnalysisDomain,
    SynopticAnalysisDomain,
    Count
};

///
/// @brief Absolute temporal support interval of one resolved ProductTimeSpec.
///
/// The domain artifact stores the start and end datetimes of the product's
/// resolved support. The start and end are absolute placements, not relative
/// durations, and are already fully computed by the selected domain builder.
///
struct ProductTimeSpecDomain {

    /// @brief Absolute start datetime of the resolved product support.
    eckit::DateTime domainStartDateTime{};

    /// @brief Absolute end datetime of the resolved product support.
    eckit::DateTime domainEndDateTime{};
};

/// @brief Return the stable diagnostic name of one domain case.
/// @param[in] value Domain classification value.
/// @return Stable human-readable domain-case name.
inline std::string productTimeSpecDomainTypeName(ProductTimeSpecDomainKind value) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        switch (value) {
            case ProductTimeSpecDomainKind::ForecastDomain:
                return "ForecastDomain";
            case ProductTimeSpecDomainKind::SeasonalForecastDomain:
                return "SeasonalForecastDomain";
            case ProductTimeSpecDomainKind::AnalysisDomain:
                return "AnalysisDomain";
            case ProductTimeSpecDomainKind::SynopticAnalysisDomain:
                return "SynopticAnalysisDomain";
        }

        return "InvalidTimeDomainKind";
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to retrieve the ProductTimeSpec domain-case name", Here()));
    }
}

/// @brief Serialize one resolved domain artifact as diagnostic JSON.
/// @param[in] value Resolved domain artifact.
/// @return One JSON object describing the final domain state.
inline std::string productTimeSpecDomainJson(const ProductTimeSpecDomain& value) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        std::ostringstream out;
        out << '{' << detail::jsonQuote_modelInput("domainStartDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.domainStartDateTime) << ','
            << detail::jsonQuote_modelInput("domainEndDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.domainEndDateTime) << '}';
        return out.str();
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to serialize the ProductTimeSpec domain artifact", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain

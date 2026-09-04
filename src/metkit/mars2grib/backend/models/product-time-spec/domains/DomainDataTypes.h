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
/// @brief Domain artifact and resolved ProductTimeSpec domain state.
///
/// This header owns:
/// - the resolved `ProductTimeSpecDomain` artifact type;
/// - the best-effort diagnostic JSON serializer for the resolved domain.
///

#pragma once

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::domain {

///
/// @brief Absolute temporal support interval of one resolved ProductTimeSpec.
///
/// The domain artifact stores the start and end datetimes of the product's
/// resolved support together with the synoptic-placement flag and the signed
/// whole-hour offsets from the anchor reference datetime.
///
/// The start and end are absolute placements, not relative durations, and are
/// already fully computed by the selected domain builder.
///
struct ProductTimeSpecDomain {

    /// @brief Absolute start datetime of the resolved product support.
    eckit::DateTime domainStartDateTime{};

    /// @brief Absolute end datetime of the resolved product support.
    eckit::DateTime domainEndDateTime{};

    /// @brief Whether the domain uses synoptic placement semantics.
    bool isSynoptic{false};

    /// @brief Signed whole-hour offset from the reference to the real support start.
    long startOffsetHoursFromReference{0};

    /// @brief Signed whole-hour offset from the reference to the support end.
    long endOffsetHoursFromReference{0};
};

/// @brief Serialize one resolved domain artifact as diagnostic JSON.
///
/// This function is best-effort and never throws. It is intended for
/// diagnostic-context construction only and therefore returns a stable fallback
/// error object if serialization fails.
///
/// @param[in] value Resolved domain artifact.
/// @return One JSON object describing the final domain state, or a stable
///         fallback error object if serialization fails.
inline std::string productTimeSpecDomainJson(const ProductTimeSpecDomain& value) noexcept {
    try {
        std::ostringstream out;
        out << '{' << detail::jsonQuote_modelInput("domainStartDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.domainStartDateTime) << ','
            << detail::jsonQuote_modelInput("domainEndDateTime") << ':'
            << detail::productTimeSpecDateTimeJson(value.domainEndDateTime) << ','
            << detail::jsonQuote_modelInput("isSynoptic") << ':' << (value.isSynoptic ? "true" : "false") << ','
            << detail::jsonQuote_modelInput("startOffsetHoursFromReference") << ':'
            << value.startOffsetHoursFromReference << ',' << detail::jsonQuote_modelInput("endOffsetHoursFromReference")
            << ':' << value.endOffsetHoursFromReference << '}';
        return out.str();
    }
    catch (...) {
        return std::string{"{\"error\":\"productTimeSpecDomainJson failed while building diagnostic context\"}"};
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain

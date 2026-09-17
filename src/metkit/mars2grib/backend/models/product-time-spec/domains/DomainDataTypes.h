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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
template <class Cntx_t>
inline std::string productTimeSpecDomainJson(const ProductTimeSpecDomain& value, Cntx_t& cntx) noexcept {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    try {
        std::ostringstream out;
        out << '{' << detail::jsonQuote_modelInput("domainStartDateTime", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
            << detail::productTimeSpecDateTimeJson(value.domainStartDateTime, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
            << detail::jsonQuote_modelInput("domainEndDateTime", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
            << detail::productTimeSpecDateTimeJson(value.domainEndDateTime, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
            << detail::jsonQuote_modelInput("isSynoptic", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << (value.isSynoptic ? "true" : "false") << ','
            << detail::jsonQuote_modelInput("startOffsetHoursFromReference", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
            << value.startOffsetHoursFromReference << ',' << detail::jsonQuote_modelInput("endOffsetHoursFromReference", metkit::mars2grib::utils::profiling::callSite(cntx, Here()))
            << ':' << value.endOffsetHoursFromReference << '}';
        {
            std::string result = out.str();
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    catch (...) {
        {
            std::string result = std::string{"{\"error\":\"productTimeSpecDomainJson failed while building diagnostic context\"}"};
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::domain

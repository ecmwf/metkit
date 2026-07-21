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
/// @file AnchorUtils.h
/// @brief Shared primitives used by ProductTimeSpec anchor matchers and builders.
///
/// The helpers in this header are limited to source-structure checks, DateTime
/// construction, and the final anchor-ordering invariant. Anchor case selection and
/// case-level construction remain in the individual anchor files.
///
/// Every function catches all failures and rethrows `Mars2GribModelException`
/// directly. Functions receiving normalized input attach `input.to_json()`.
///
/// @ingroup mars2grib_product_time_spec_detail
///
#pragma once

#include "metkit/mars2grib/utils/TemporalArithmetic.h"

namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail {

///
/// @brief Test whether both MARS year and month are available.
///
/// The pair is indivisible forecast-reference input.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` when both `marsYear` and `marsMonth` are present.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool hasCompleteYearMonth(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasYear  = input.marsYear.has_value();
        const bool hasMonth = input.marsMonth.has_value();

        return hasYear && hasMonth;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to test the complete MARS year/month pair", input.to_json(), Here()));
    }
}

///
/// @brief Test whether exactly one member of the MARS year/month pair is present.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return `true` for an incomplete year/month pair; otherwise `false`.
/// @throws Mars2GribModelException If the check cannot be completed.
///
inline bool hasPartialYearMonth(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool hasYear  = input.marsYear.has_value();
        const bool hasMonth = input.marsMonth.has_value();

        return hasYear != hasMonth;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to test the partial MARS year/month pair", input.to_json(), Here()));
    }
}

///
/// @brief Construct the ForecastAnalysis anchor datetime from MARS date and optional time.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return DateTime derived from MARS date/time.
/// @throws Mars2GribModelException If date is missing or construction fails.
///
inline eckit::DateTime forecastAnalysisDateTimeFromInput(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        const bool hasMarsDate = input.marsDate.has_value();

        if (!hasMarsDate) {
            throw Mars2GribModelException("ForecastAnalysis anchor construction requires MARS date", input.to_json(),
                                          Here());
        }

        return makeDateTime(*input.marsDate, input.marsTime);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to construct ForecastAnalysis anchor datetime", input.to_json(), Here()));
    }
}

///
/// @brief Construct the Hindcast label datetime from `hdate` at the default time.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return DateTime constructed from `marsHdate` at `00:00:00`.
/// @throws Mars2GribModelException If hdate is missing or construction fails.
///
inline eckit::DateTime hindcastLabelDateTimeFromInput(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        const bool hasHdate = input.marsHdate.has_value();

        if (!hasHdate) {
            throw Mars2GribModelException("Hindcast anchor construction requires hdate", input.to_json(), Here());
        }

        return makeDateTime(*input.marsHdate, defaultMarsTime());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to construct Hindcast labelDateTime", input.to_json(), Here()));
    }
}

///
/// @brief Construct the Hindcast initial-conditions datetime from MARS date and optional time.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return DateTime derived from MARS date/time.
/// @throws Mars2GribModelException If date is missing or construction fails.
///
inline eckit::DateTime hindcastInitialConditionsDateTimeFromInput(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        const bool hasMarsDate = input.marsDate.has_value();

        if (!hasMarsDate) {
            throw Mars2GribModelException("Hindcast anchor construction requires MARS date", input.to_json(), Here());
        }

        return makeDateTime(*input.marsDate, input.marsTime);
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to construct Hindcast initialConditionsDateTime", input.to_json(), Here()));
    }
}

///
/// @brief Construct the SeasonalClimate anchor datetime from MARS year and month.
///
/// The resulting datetime is the first day of the requested month at midnight.
///
/// @param[in] input Fully normalized ProductTimeSpec input.
/// @return First instant of the requested year/month.
/// @throws Mars2GribModelException If year/month is incomplete or construction fails.
///
inline eckit::DateTime seasonalClimateDateTimeFromInput(const ProductTimeSpecInput& input) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        const bool hasForecastYearMonth = hasCompleteYearMonth(input);

        if (!hasForecastYearMonth) {
            throw Mars2GribModelException("SeasonalClimate anchor construction requires MARS year and month",
                                          input.to_json(), Here());
        }

        const eckit::Date firstDayOfMonth{*input.marsYear, *input.marsMonth, 1};

        return makeDateTime(firstDayOfMonth, defaultMarsTime());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to construct SeasonalClimate anchor datetime", input.to_json(), Here()));
    }
}

///
/// @brief Validate anchor ordering and construct a ProductTimeSpec anchor.
///
/// The canonical invariant is:
/// `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
///
/// @param[in] input Fully normalized input attached to any exception.
/// @param[in] label Candidate label DateTime.
/// @param[in] initialConditions Candidate initial-condition DateTime.
/// @param[in] reference Candidate reference DateTime.
/// @param[in] anchorType Resolved anchor classification.
/// @return Complete and order-validated ProductTimeSpec anchor.
/// @throws Mars2GribModelException If ordering is invalid or construction fails.
///
inline ProductTimeSpecAnchor checkedAnchor(const ProductTimeSpecInput& input, const eckit::DateTime& label,
                                           const eckit::DateTime& initialConditions, const eckit::DateTime& reference,
                                           anchor::ProductTimeSpecAnchorKind anchorType) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const bool labelDoesNotFollowInitialConditions   = label <= initialConditions;
        const bool initialConditionsDoNotFollowReference = initialConditions <= reference;
        const bool orderingIsValid = labelDoesNotFollowInitialConditions && initialConditionsDoNotFollowReference;

        if (!orderingIsValid) {
            throw Mars2GribModelException("Anchor ordering must satisfy label <= initial conditions <= reference",
                                          input.to_json(), Here());
        }

        return ProductTimeSpecAnchor{label, initialConditions, reference, anchorType};
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException("Failed to validate and construct ProductTimeSpec anchor",
                                                       input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::anchor::detail

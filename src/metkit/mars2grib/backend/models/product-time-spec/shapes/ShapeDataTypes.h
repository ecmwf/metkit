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
/// @file ShapeDataTypes.h
/// @brief Shape classifications and resolved ProductTimeSpec window artifacts.
///

#pragma once

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape {

///
/// @brief One canonical ProductTimeSpec statistical window.
///
/// Each canonical window stores the statistical processing applied over that
/// window, the GRIB `typeOfTimeIncrement` associated with it, its range, and the
/// increment associated with samples contributing to it.
///
struct ProductTimeSpecWindow {
    /// @brief Statistical processing performed over this canonical window.
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing{tables::TypeOfStatisticalProcessing::Missing};

    /// @brief GRIB `typeOfTimeIncrement` describing the increment semantics.
    metkit::mars2grib::backend::tables::TypeOfTimeIntervals typeOfTimeIncrement{
        metkit::mars2grib::backend::tables::TypeOfTimeIntervals::Missing};

    /// @brief Length of the canonical statistical window.
    deductions::TimeDuration timeRange{};

    /// @brief Increment associated with samples contributing to the window.
    deductions::TimeDuration timeIncrement{};
};

///
/// @brief Ordered canonical ProductTimeSpec window sequence.
///
/// Windows are stored in outermost-to-innermost order.
///
struct ProductTimeSpecShape {
    /// @brief Canonical windows in outermost-to-innermost order.
    std::vector<ProductTimeSpecWindow> values{};
};

/// @brief Serialize one resolved shape artifact as diagnostic JSON.
/// @param[in] value Resolved canonical window sequence.
/// @return One JSON object describing the final shape state.
inline std::string productTimeSpecShapeJson(const ProductTimeSpecShape& value) {
    std::ostringstream out;
    out << '{' << detail::jsonQuote_modelInput("windows") << ':' << '[';
    for (std::size_t i = 0; i < value.values.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << '{' << detail::jsonQuote_modelInput("typeOfStatisticalProcessing") << ':'
            << detail::jsonQuote_modelInput(
                   tables::enum2name_TypeOfStatisticalProcessing_or_throw(value.values[i].typeOfStatisticalProcessing))
            << ',' << detail::jsonQuote_modelInput("typeOfTimeIncrement") << ':'
            << detail::jsonQuote_modelInput(
                   tables::enum2name_TypeOfTimeIntervals_or_throw(value.values[i].typeOfTimeIncrement))
            << ',' << detail::jsonQuote_modelInput("timeRange") << ':'
            << detail::durationJson_modelInput(value.values[i].timeRange) << ','
            << detail::jsonQuote_modelInput("timeIncrement") << ':'
            << detail::durationJson_modelInput(value.values[i].timeIncrement) << '}';
    }
    out << ']' << '}';
    return out.str();
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape

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
/// @file ProductTimeSpecJsonUtils.h
/// @brief Internal JSON helpers for ProductTimeSpec model-input diagnostics.
///

#pragma once

#include <optional>
#include <sstream>
#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/stattype.h"
#include "metkit/mars2grib/backend/deductions/timespan.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

namespace metkit::mars2grib::backend::models::detail {

inline std::string jsonQuote_modelInput(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        switch (c) {
            case '\\':
                out << "\\\\";
                break;
            case '"':
                out << "\\\"";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                out << static_cast<char>(c);
                break;
        }
    }
    out << '"';
    return out.str();
}

inline std::string optionalDateTimeJson_modelInput(const std::optional<eckit::DateTime>& value) {
    if (!value.has_value()) {
        return "null";
    }
    return jsonQuote_modelInput(value->iso(true));
}

inline std::string durationJson_modelInput(const deductions::TimeDuration& value) {
    std::ostringstream out;
    out << '{' << jsonQuote_modelInput("length") << ':' << value.length << ','
        << jsonQuote_modelInput("unit") << ':' << jsonQuote_modelInput(tables::enum2name_TimeUnit_or_throw(value.unit))
        << '}';
    return out.str();
}

inline std::string optionalDurationJson_modelInput(const std::optional<deductions::TimeDuration>& value) {
    return value.has_value() ? durationJson_modelInput(*value) : std::string{"null"};
}

inline std::string timespanKindName_modelInput(deductions::TimespanKind value) {
    switch (value) {
        case deductions::TimespanKind::Missing:
            return "missing";
        case deductions::TimespanKind::Duration:
            return "duration";
        case deductions::TimespanKind::None:
            return "none";
        case deductions::TimespanKind::FromStart:
            return "from-start";
    }

    return "invalid";
}

inline std::string optionalTimespanJson_modelInput(const std::optional<deductions::Timespan>& value) {
    if (!value.has_value()) {
        return "null";
    }

    std::ostringstream out;
    out << '{' << jsonQuote_modelInput("kind") << ':' << jsonQuote_modelInput(timespanKindName_modelInput(value->kind))
        << ',' << jsonQuote_modelInput("duration") << ':'
        << (value->duration.has_value() ? durationJson_modelInput(*value->duration) : std::string{"null"}) << '}';
    return out.str();
}

inline std::string parsedStattypeBlocksJson_modelInput(const deductions::ParsedStatTypeBlocks& value) {
    std::ostringstream out;
    out << '[';
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            out << ',';
        }

        out << '{' << jsonQuote_modelInput("timeRange") << ':' << durationJson_modelInput(value[i].timeRange) << ','
            << jsonQuote_modelInput("typeOfStatisticalProcessing") << ':'
            << jsonQuote_modelInput(tables::enum2name_TypeOfStatisticalProcessing_or_throw(value[i].typeOfStatisticalProcessing))
            << '}';
    }
    out << ']';
    return out.str();
}

inline std::string optionalStattypeJson_modelInput(const std::optional<deductions::ParsedStatTypeBlocks>& value) {
    return value.has_value() ? parsedStattypeBlocksJson_modelInput(*value) : std::string{"null"};
}

}  // namespace metkit::mars2grib::backend::models::detail

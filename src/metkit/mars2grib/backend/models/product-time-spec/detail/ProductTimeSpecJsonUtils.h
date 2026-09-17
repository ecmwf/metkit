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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include <optional>
#include <sstream>
#include <string>

#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/stattype.h"
#include "metkit/mars2grib/backend/deductions/timespan.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

namespace metkit::mars2grib::backend::models::product_time_spec::detail {

template <class Cntx_t>
inline std::string jsonQuote_modelInput(const std::string& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
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
    {
        std::string result = out.str();
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string productTimeSpecDateTimeJson(const eckit::DateTime& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    {
        std::string result = jsonQuote_modelInput(value.iso(true), metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string optionalDateTimeJson_modelInput(const std::optional<eckit::DateTime>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    if (!value.has_value()) {
        {
            std::string result = "null";
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    {
        std::string result = jsonQuote_modelInput(value->iso(true), metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

/// @brief Serialize an optional `eckit::Date` as JSON.
/// @param[in] value Optional normalized date value.
/// @return JSON `null` when absent, otherwise one quoted date string.
template <class Cntx_t>
inline std::string optionalDateJson_modelInput(const std::optional<eckit::Date>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    if (!value.has_value()) {
        {
            std::string result = "null";
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    {
        std::string result = std::to_string(value->yyyymmdd());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

/// @brief Serialize an optional `eckit::Time` as JSON.
/// @param[in] value Optional normalized time value.
/// @return JSON `null` when absent, otherwise one quoted time string.
template <class Cntx_t>
inline std::string optionalTimeJson_modelInput(const std::optional<eckit::Time>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    if (!value.has_value()) {
        {
            std::string result = "null";
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    {
        std::string result = std::to_string(value->hhmmss());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

/// @brief Serialize an optional `long` as JSON.
/// @param[in] value Optional normalized integer value.
/// @return JSON `null` when absent, otherwise one decimal integer.
template <class Cntx_t>
inline std::string optionalLongJson_modelInput(const std::optional<long>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    {
        std::string result = value.has_value() ? std::to_string(*value) : std::string{"null"};
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string durationJson_modelInput(const deductions::TimeDuration& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    std::ostringstream out;
    out << '{' << jsonQuote_modelInput("length", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << value.length << ',' << jsonQuote_modelInput("unit", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
        << jsonQuote_modelInput(tables::enum2name_TimeUnit_or_throw(value.unit, metkit::mars2grib::utils::profiling::callSite(cntx, Here())), metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << '}';
    {
        std::string result = out.str();
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string optionalDurationJson_modelInput(const std::optional<deductions::TimeDuration>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    {
        std::string result = value.has_value() ? durationJson_modelInput(*value, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) : std::string{"null"};
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string timespanKindName_modelInput(deductions::TimespanKind value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    switch (value) {
        case deductions::TimespanKind::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case deductions::TimespanKind::Duration:
            {
                std::string result = "duration";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case deductions::TimespanKind::None:
            {
                std::string result = "none";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case deductions::TimespanKind::FromStart:
            {
                std::string result = "from-start";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
    }

    {
        std::string result = "invalid";
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string optionalTimespanJson_modelInput(const std::optional<deductions::Timespan>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    if (!value.has_value()) {
        {
            std::string result = "null";
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    std::ostringstream out;
    out << '{' << jsonQuote_modelInput("kind", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << jsonQuote_modelInput(timespanKindName_modelInput(value->kind, metkit::mars2grib::utils::profiling::callSite(cntx, Here())), metkit::mars2grib::utils::profiling::callSite(cntx, Here()))
        << ',' << jsonQuote_modelInput("duration", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
        << (value->duration.has_value() ? durationJson_modelInput(*value->duration, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) : std::string{"null"}) << '}';
    {
        std::string result = out.str();
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string parsedStattypeBlocksJson_modelInput(const deductions::ParsedStatTypeBlocks& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    std::ostringstream out;
    out << '[';
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i != 0) {
            out << ',';
        }

        out << '{' << jsonQuote_modelInput("timeRange", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << durationJson_modelInput(value[i].timeRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
            << jsonQuote_modelInput("typeOfStatisticalProcessing", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
            << jsonQuote_modelInput(
                   tables::enum2name_TypeOfStatisticalProcessing_or_throw(value[i].typeOfStatisticalProcessing, metkit::mars2grib::utils::profiling::callSite(cntx, Here())), metkit::mars2grib::utils::profiling::callSite(cntx, Here()))
            << '}';
    }
    out << ']';
    {
        std::string result = out.str();
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

template <class Cntx_t>
inline std::string optionalStattypeJson_modelInput(const std::optional<deductions::ParsedStatTypeBlocks>& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    {
        std::string result = value.has_value() ? parsedStattypeBlocksJson_modelInput(*value, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) : std::string{"null"};
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::detail

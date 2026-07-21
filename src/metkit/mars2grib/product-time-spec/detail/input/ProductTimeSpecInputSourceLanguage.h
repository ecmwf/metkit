/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file ProductTimeSpecInputSourceLanguage.h
 * @brief Source-language policy helpers for timespan and stattype parsing.
 */

#pragma once

#include <array>
#include <string>
#include <string_view>

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputCommon.h"

namespace metkit::mars2grib::product_time_spec::input_detail {

inline bool isSupportedTimespanHours(long hours) {
    constexpr std::array<long, 12> supported{
        1, 3, 6, 12, 18, 24, 48, 72, 120, 168, 240, 360
    };
    return std::find(supported.begin(), supported.end(), hours) != supported.end();
}

inline bool isSupportedTimespanString(const std::string& value) {
    constexpr std::array<std::string_view, 12> supported{
        "1h", "3h", "6h", "12h", "18h", "24h", "48h", "72h",
        "120h", "168h", "240h", "360h"
    };
    return std::find(supported.begin(), supported.end(), value) != supported.end();
}

inline bool isRecognizedUnsupportedTimespan(const std::string& value) {
    return value == "inst" || value == "instantaneous" || value == "10m" ||
           value == "15m" || value == "20m" || value == "30m";
}

inline bool isWhitelistedStatType(const std::string& value) {
    constexpr std::array<std::string_view, 25> whitelist{
        "moav", "momn", "momx", "mosd", "daac", "daav", "damn", "damx", "dasd",
        "moav_daav", "moav_damn", "moav_damx", "moav_dasd",
        "momn_daav", "momn_damn", "momn_damx", "momn_dasd",
        "momx_daav", "momx_damn", "momx_damx", "momx_dasd",
        "mosd_daav", "mosd_damn", "mosd_damx", "mosd_dasd"
    };
    return std::find(whitelist.begin(), whitelist.end(), value) != whitelist.end();
}

inline tables::TypeOfStatisticalProcessing parseStatOperation(const std::string& operation) {
    if (operation == "av") return tables::TypeOfStatisticalProcessing::Average;
    if (operation == "ac") return tables::TypeOfStatisticalProcessing::Accumulation;
    if (operation == "mn") return tables::TypeOfStatisticalProcessing::Minimum;
    if (operation == "mx") return tables::TypeOfStatisticalProcessing::Maximum;
    if (operation == "sd") return tables::TypeOfStatisticalProcessing::StandardDeviation;
    throw Mars2GribGenericException("Unsupported stattype operation: '" + operation + "'", Here());
}

inline ParsedStatTypeBlocks parseStatType(const std::string& raw) {
    const std::string value = lower(raw);
    if (!isWhitelistedStatType(value)) {
        throw Mars2GribGenericException("`stattype` is not in the supported MARS whitelist: '" + raw + "'", Here());
    }

    ParsedStatTypeBlocks result;
    std::size_t start = 0;
    while (start < value.size()) {
        const std::size_t end = value.find('_', start);
        const std::string block = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (block.size() != 4) {
            throw Mars2GribGenericException("Invalid stattype block: '" + block + "'", Here());
        }

        const std::string period = block.substr(0, 2);
        const std::string operation = block.substr(2, 2);
        ProductTimeDuration range;
        if (period == "mo") {
            range = ProductTimeDuration{tables::TimeUnit::Month, 1};
        } else if (period == "da") {
            range = ProductTimeDuration{tables::TimeUnit::Day, 1};
        } else {
            throw Mars2GribGenericException("Unsupported stattype period: '" + period + "'", Here());
        }
        result.append(ParsedStatTypeBlock{range, parseStatOperation(operation)});

        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return result;
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail

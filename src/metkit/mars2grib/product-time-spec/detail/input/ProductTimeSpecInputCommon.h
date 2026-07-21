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
 * @file ProductTimeSpecInputCommon.h
 * @brief Shared lexical helpers for ProductTimeSpecInput extraction.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>

#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::product_time_spec::input_detail {

using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

inline std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

inline std::string digitsOnly(std::string value, char ignored) {
    value.erase(std::remove(value.begin(), value.end(), ignored), value.end());
    return value;
}

inline long parseLongStrict(const std::string& value, const std::string& key) {
    std::size_t used = 0;
    long result = 0;
    try {
        result = std::stol(value, &used);
    } catch (...) {
        throw Mars2GribGenericException("Invalid integer value for `" + key + "`: '" + value + "'", Here());
    }
    if (used != value.size()) {
        throw Mars2GribGenericException("Invalid trailing characters in `" + key + "`: '" + value + "'", Here());
    }
    return result;
}

inline long checkedHoursToSeconds(long hours, const std::string& key) {
    if (hours > std::numeric_limits<long>::max() / 3600L ||
        hours < std::numeric_limits<long>::min() / 3600L) {
        throw Mars2GribGenericException("Duration overflow while converting `" + key + "` from hours", Here());
    }
    return hours * 3600L;
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail

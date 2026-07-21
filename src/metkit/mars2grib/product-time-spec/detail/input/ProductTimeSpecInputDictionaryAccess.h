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
 * @file ProductTimeSpecInputDictionaryAccess.h
 * @brief Dictionary-facing normalization helpers for ProductTimeSpecInput.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputCommon.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

namespace metkit::mars2grib::product_time_spec::input_detail {

template <typename Dict>
std::optional<long> optionalLong(const Dict& dict, std::string_view key) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    if (!has(dict, key)) {
        return std::nullopt;
    }
    if (auto value = get_opt<long>(dict, key)) {
        return value;
    }
    if (auto value = get_opt<std::string>(dict, key)) {
        return parseLongStrict(*value, std::string(key));
    }
    throw Mars2GribGenericException("Key `" + std::string(key) + "` is neither integer nor integer string", Here());
}

template <typename Dict>
std::optional<std::string> optionalString(const Dict& dict, std::string_view key) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    if (!has(dict, key)) {
        return std::nullopt;
    }
    if (auto value = get_opt<std::string>(dict, key)) {
        return value;
    }
    throw Mars2GribGenericException("Key `" + std::string(key) + "` is not a string", Here());
}

template <typename Dict>
long mandatoryLong(const Dict& dict, std::string_view key) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    return get_or_throw<long>(dict, key);
}

template <typename Dict>
std::string mandatoryString(const Dict& dict, std::string_view key) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    return lower(get_or_throw<std::string>(dict, key));
}

template <typename Dict>
bool optionBool(const Dict& dict, std::string_view key, bool defaultValue = false) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    if (!has(dict, key)) {
        return defaultValue;
    }
    if (auto value = get_opt<bool>(dict, key)) {
        return *value;
    }
    if (auto value = get_opt<long>(dict, key)) {
        if (*value == 0 || *value == 1) {
            return *value == 1;
        }
    }
    if (auto value = get_opt<std::string>(dict, key)) {
        const std::string normalized = lower(*value);
        if (normalized == "true" || normalized == "yes" || normalized == "1") return true;
        if (normalized == "false" || normalized == "no" || normalized == "0") return false;
    }
    throw Mars2GribGenericException("Invalid boolean option `" + std::string(key) + "`", Here());
}

inline TypeOfTimeIncrement parseTypeOfTimeIncrementName(const std::string& raw) {
    const std::string value = lower(raw);
    if (value == "reserved") return TypeOfTimeIncrement::Reserved;
    if (value == "same-forecast-time-start-incremented") return TypeOfTimeIncrement::SameForecastTimeStartIncremented;
    if (value == "same-start-time-forecast-incremented") return TypeOfTimeIncrement::SameStartTimeForecastIncremented;
    if (value == "start-incremented-forecast-decremented-constant-valid") {
        return TypeOfTimeIncrement::StartIncrementedForecastDecrementedConstantValid;
    }
    if (value == "start-decremented-forecast-incremented-constant-valid") {
        return TypeOfTimeIncrement::StartDecrementedForecastIncrementedConstantValid;
    }
    if (value == "floating-subinterval") return TypeOfTimeIncrement::FloatingSubinterval;
    if (value == "missing") return TypeOfTimeIncrement::Missing;
    throw Mars2GribGenericException("Invalid `defaultTypeOfTimeIncrement`: '" + raw + "'", Here());
}

inline TypeOfTimeIncrement parseTypeOfTimeIncrementLong(long value) {
    switch (value) {
        case 0: return TypeOfTimeIncrement::Reserved;
        case 1: return TypeOfTimeIncrement::SameForecastTimeStartIncremented;
        case 2: return TypeOfTimeIncrement::SameStartTimeForecastIncremented;
        case 3: return TypeOfTimeIncrement::StartIncrementedForecastDecrementedConstantValid;
        case 4: return TypeOfTimeIncrement::StartDecrementedForecastIncrementedConstantValid;
        case 5: return TypeOfTimeIncrement::FloatingSubinterval;
        case 255: return TypeOfTimeIncrement::Missing;
        default:
            throw Mars2GribGenericException("Invalid numeric `defaultTypeOfTimeIncrement`: '" +
                                            std::to_string(value) + "'", Here());
    }
}

template <typename Dict>
TypeOfTimeIncrement optionTypeOfTimeIncrement(const Dict& dict) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    constexpr std::string_view key = "defaultTypeOfTimeIncrement";
    if (!has(dict, key)) {
        return TypeOfTimeIncrement::Missing;
    }
    if (auto value = get_opt<long>(dict, key)) {
        return parseTypeOfTimeIncrementLong(*value);
    }
    if (auto value = get_opt<std::string>(dict, key)) {
        return parseTypeOfTimeIncrementName(*value);
    }
    throw Mars2GribGenericException("Invalid option type for `defaultTypeOfTimeIncrement`", Here());
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail

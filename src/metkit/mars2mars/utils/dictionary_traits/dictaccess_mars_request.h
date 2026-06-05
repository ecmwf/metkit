#pragma once

#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "eckit/log/JSON.h"

#include "metkit/mars/MarsRequest.h"

#include "metkit/mars2mars/utils/generalUtils.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/type_traits_name.h"

namespace metkit::mars2mars::utils {

template <>
constexpr std::string_view type_name<metkit::mars::MarsRequest>() {
    return "metkit::mars::MarsRequest";
}

}  // namespace metkit::mars2mars::utils

namespace metkit::mars2mars::utils::dict_traits {

using std::operator""s;

namespace detail {

inline const std::string& scalarStringOrThrow(
    const metkit::mars::MarsRequest& mars,
    std::string_view key) {

    const std::string k{key};

    if (!mars.has(k)) {
        throw exceptions::Mars2marsDictException(
            "Missing key `"s + k + "` in metkit::mars::MarsRequest",
            Here());
    }

    const auto& values = mars.values(k);

    if (values.size() != 1) {
        throw exceptions::Mars2marsDictException(
            "Invalid non-scalar key `"s + k + "` in metkit::mars::MarsRequest: found `"s +
                std::to_string(values.size()) +
                "` values. mars2mars expects a single MARS point, not a hypercube.",
            Here());
    }

    return values.front();
}

inline std::optional<std::string> scalarStringOpt(
    const metkit::mars::MarsRequest& mars,
    std::string_view key) {

    const std::string k{key};

    if (!mars.has(k)) {
        return std::nullopt;
    }

    const auto& values = mars.values(k);

    if (values.empty()) {
        return std::nullopt;
    }

    if (values.size() != 1) {
        throw exceptions::Mars2marsDictException(
            "Invalid non-scalar key `"s + k + "` in metkit::mars::MarsRequest: found `"s +
                std::to_string(values.size()) +
                "` values. mars2mars expects a single MARS point, not a hypercube.",
            Here());
    }

    return values.front();
}

inline long parsePlainLong(std::string_view value, std::string_view key) {
    const std::string s{value};

    std::size_t pos = 0;
    long result = 0;

    try {
        result = std::stol(s, &pos);
    }
    catch (...) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to long",
            Here());
    }

    if (pos != s.size()) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s +
                "` to long without loss",
            Here());
    }

    return result;
}

inline double parsePlainDouble(std::string_view value, std::string_view key) {
    const std::string s{value};

    std::size_t pos = 0;
    double result = 0.0;

    try {
        result = std::stod(s, &pos);
    }
    catch (...) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to double",
            Here());
    }

    if (pos != s.size()) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s +
                "` to double without loss",
            Here());
    }

    return result;
}

inline long parseHoursAsLong(std::string_view value, std::string_view key) {
    const std::string s{value};

    if (s.empty()) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert empty key `"s + std::string{key} + "` value to hours",
            Here());
    }

    if (s.back() == 'h') {
        return parsePlainLong(std::string_view{s.data(), s.size() - 1}, key);
    }

    if (s.back() == 'm' || s.back() == 's') {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s +
                "` to hours: sub-hourly extents are not supported by this mars2mars path",
            Here());
    }

    // Existing workflows treat bare numeric step/timespan as hours.
    return parsePlainLong(s, key);
}

inline std::string longToHours(long value) {
    return std::to_string(value) + "h";
}

inline std::string doubleToString(double value) {
    std::ostringstream os;
    os.precision(std::numeric_limits<double>::max_digits10);
    os << value;
    return os.str();
}

// -----------------------------------------------------------------------------
// long converters
// -----------------------------------------------------------------------------

using ToLong   = long (*)(std::string_view value, std::string_view key);
using FromLong = std::string (*)(long value);

inline long paramToLong(std::string_view value, std::string_view key) {
    // Conservative first implementation:
    //   param=130      -> OK
    //   param=228.128  -> throw
    //
    // If later you need "228.128" -> some canonical long encoding,
    // add that rule here, not in generic parsing.
    return parsePlainLong(value, key);
}

inline long plainLong(std::string_view value, std::string_view key) {
    return parsePlainLong(value, key);
}

inline long hoursLong(std::string_view value, std::string_view key) {
    return parseHoursAsLong(value, key);
}

inline std::string plainLongString(long value) {
    return std::to_string(value);
}

inline const std::unordered_map<std::string, ToLong>& toLongConverters() {
    static const std::unordered_map<std::string, ToLong> converters = {
        {"param", paramToLong},
        {"levelist", plainLong},
        {"chem", plainLong},
        {"step", hoursLong},
        {"timespan", hoursLong},
    };

    return converters;
}

inline const std::unordered_map<std::string, FromLong>& fromLongConverters() {
    static const std::unordered_map<std::string, FromLong> converters = {
        {"param", plainLongString},
        {"levelist", plainLongString},
        {"chem", plainLongString},
        {"step", longToHours},
        {"timespan", longToHours},
    };

    return converters;
}

inline long convertToLongOrThrow(
    std::string_view key,
    std::string_view value) {

    const std::string k{key};

    const auto& converters = toLongConverters();
    const auto it = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support conversion to long for metkit::mars::MarsRequest",
            Here());
    }

    return it->second(value, key);
}

inline std::string convertFromLongOrThrow(
    std::string_view key,
    long value) {

    const std::string k{key};

    const auto& converters = fromLongConverters();
    const auto it = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support assignment from long for metkit::mars::MarsRequest",
            Here());
    }

    return it->second(value);
}

// -----------------------------------------------------------------------------
// double converters
// -----------------------------------------------------------------------------

using ToDouble   = double (*)(std::string_view value, std::string_view key);
using FromDouble = std::string (*)(double value);

inline const std::unordered_map<std::string, ToDouble>& toDoubleConverters() {
    static const std::unordered_map<std::string, ToDouble> converters = {
        {"wavelength", parsePlainDouble},
    };

    return converters;
}

inline const std::unordered_map<std::string, FromDouble>& fromDoubleConverters() {
    static const std::unordered_map<std::string, FromDouble> converters = {
        {"wavelength", doubleToString},
    };

    return converters;
}

inline double convertToDoubleOrThrow(
    std::string_view key,
    std::string_view value) {

    const std::string k{key};

    const auto& converters = toDoubleConverters();
    const auto it = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support conversion to double for metkit::mars::MarsRequest",
            Here());
    }

    return it->second(value, key);
}

inline std::string convertFromDoubleOrThrow(
    std::string_view key,
    double value) {

    const std::string k{key};

    const auto& converters = fromDoubleConverters();
    const auto it = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support assignment from double for metkit::mars::MarsRequest",
            Here());
    }

    return it->second(value);
}

}  // namespace detail

// -----------------------------------------------------------------------------
// DictToJsonTraits
// -----------------------------------------------------------------------------

template <>
struct DictToJsonTraits<metkit::mars::MarsRequest> {
    static std::string to_json(const metkit::mars::MarsRequest& mars) noexcept(true) {
        try {
            std::ostringstream os;
            eckit::JSON json(os);
            mars.json(json);
            return os.str();
        }
        catch (...) {
            return "[to_json failed for metkit::mars::MarsRequest]";
        }
    }
};

// -----------------------------------------------------------------------------
// DictTraits
// -----------------------------------------------------------------------------

template <>
struct DictTraits<metkit::mars::MarsRequest> {
    static constexpr bool support_checks = false;

    // static std::unique_ptr<metkit::mars::MarsRequest>
    // make_from_sample_or_throw(std::string_view name) {
    //     return std::make_unique<metkit::mars::MarsRequest>(std::string{name});
    // }

    static std::unique_ptr<metkit::mars::MarsRequest>
    clone_or_throw(const metkit::mars::MarsRequest& mars) {
        return std::make_unique<metkit::mars::MarsRequest>(mars);
    }
};

// -----------------------------------------------------------------------------
// DictHas
// -----------------------------------------------------------------------------

template <>
struct DictHas<metkit::mars::MarsRequest> {
    static bool has(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {
        return mars.has(std::string{key});
    }
};

// -----------------------------------------------------------------------------
// DictMissing
// -----------------------------------------------------------------------------

template <>
struct DictMissing<metkit::mars::MarsRequest> {
    static bool isMissing(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {
        const std::string k{key};

        if (!mars.has(k)) {
            return true;
        }

        const auto& values = mars.values(k, true);
        return values.empty();
    }

    static void setMissing(metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {
        mars.unsetValues(std::string{key});
    }
};

// -----------------------------------------------------------------------------
// std::string access: all keys, raw scalar string
// -----------------------------------------------------------------------------

template <>
struct DictGetOrThrow<metkit::mars::MarsRequest, std::string> {
    static std::string get_or_throw(
        const metkit::mars::MarsRequest& mars,
        std::string_view key) noexcept(false) {

        return detail::scalarStringOrThrow(mars, key);
    }
};

template <>
struct DictGetOpt<metkit::mars::MarsRequest, std::string> {
    static std::optional<std::string> get_opt(
        const metkit::mars::MarsRequest& mars,
        std::string_view key) noexcept(false) {

        return detail::scalarStringOpt(mars, key);
    }
};

template <>
struct DictSetOrThrow<metkit::mars::MarsRequest, std::string> {
    static void set_or_throw(
        metkit::mars::MarsRequest& mars,
        std::string_view key,
        const std::string& value) noexcept(false) {

        mars.setValue(std::string{key}, value);
    }
};

template <>
struct DictSetOrIgnore<metkit::mars::MarsRequest, std::string> {
    static void set_or_ignore(
        metkit::mars::MarsRequest& mars,
        std::string_view key,
        const std::string& value) noexcept(false) {

        try {
            mars.setValue(std::string{key}, value);
        }
        catch (...) {
        }
    }
};

// -----------------------------------------------------------------------------
// long access: explicit supported keys only
// -----------------------------------------------------------------------------

template <>
struct DictGetOrThrow<metkit::mars::MarsRequest, long> {
    static long get_or_throw(
        const metkit::mars::MarsRequest& mars,
        std::string_view key) noexcept(false) {

        const auto& raw = detail::scalarStringOrThrow(mars, key);
        return detail::convertToLongOrThrow(key, raw);
    }
};

template <>
struct DictGetOpt<metkit::mars::MarsRequest, long> {
    static std::optional<long> get_opt(
        const metkit::mars::MarsRequest& mars,
        std::string_view key) noexcept(false) {

        const auto raw = detail::scalarStringOpt(mars, key);

        if (!raw.has_value()) {
            return std::nullopt;
        }

        return detail::convertToLongOrThrow(key, *raw);
    }
};

template <>
struct DictSetOrThrow<metkit::mars::MarsRequest, long> {
    static void set_or_throw(
        metkit::mars::MarsRequest& mars,
        std::string_view key,
        const long& value) noexcept(false) {

        mars.setValue(std::string{key}, detail::convertFromLongOrThrow(key, value));
    }
};

template <>
struct DictSetOrIgnore<metkit::mars::MarsRequest, long> {
    static void set_or_ignore(
        metkit::mars::MarsRequest& mars,
        std::string_view key,
        const long& value) noexcept(false) {

        try {
            mars.setValue(std::string{key}, detail::convertFromLongOrThrow(key, value));
        }
        catch (...) {
        }
    }
};

// -----------------------------------------------------------------------------
// double access: wavelength only
// -----------------------------------------------------------------------------

template <>
struct DictGetOrThrow<metkit::mars::MarsRequest, double> {
    static double get_or_throw(
        const metkit::mars::MarsRequest& mars,
        std::string_view key) noexcept(false) {

        const auto& raw = detail::scalarStringOrThrow(mars, key);
        return detail::convertToDoubleOrThrow(key, raw);
    }
};

template <>
struct DictGetOpt<metkit::mars::MarsRequest, double> {
    static std::optional<double> get_opt(
        const metkit::mars::MarsRequest& mars,
        std::string_view key) noexcept(false) {

        const auto raw = detail::scalarStringOpt(mars, key);

        if (!raw.has_value()) {
            return std::nullopt;
        }

        return detail::convertToDoubleOrThrow(key, *raw);
    }
};

template <>
struct DictSetOrThrow<metkit::mars::MarsRequest, double> {
    static void set_or_throw(
        metkit::mars::MarsRequest& mars,
        std::string_view key,
        const double& value) noexcept(false) {

        mars.setValue(std::string{key}, detail::convertFromDoubleOrThrow(key, value));
    }
};

template <>
struct DictSetOrIgnore<metkit::mars::MarsRequest, double> {
    static void set_or_ignore(
        metkit::mars::MarsRequest& mars,
        std::string_view key,
        const double& value) noexcept(false) {

        try {
            mars.setValue(std::string{key}, detail::convertFromDoubleOrThrow(key, value));
        }
        catch (...) {
        }
    }
};

}  // namespace metkit::mars2mars::utils::dict_traits
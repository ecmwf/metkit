#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "eckit/log/JSON.h"

#include "metkit/mars/MarsRequest.h"

#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/generalUtils.h"
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

inline const std::string& scalarStringOrThrow(const metkit::mars::MarsRequest& mars, std::string_view key) {

    const std::string k{key};

    if (!mars.has(k)) {
        throw exceptions::Mars2marsDictException("Missing key `"s + k + "` in metkit::mars::MarsRequest", Here());
    }

    const auto& values = mars.values(k);

    if (values.size() != 1) {
        throw exceptions::Mars2marsDictException(
            "Invalid non-scalar key `"s + k + "` in metkit::mars::MarsRequest: found `"s +
                std::to_string(values.size()) + "` values. mars2mars expects a single MARS point, not a hypercube.",
            Here());
    }

    return values.front();
}

inline std::optional<std::string> scalarStringOpt(const metkit::mars::MarsRequest& mars, std::string_view key) {

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
                std::to_string(values.size()) + "` values. mars2mars expects a single MARS point, not a hypercube.",
            Here());
    }

    return values.front();
}

inline long parsePlainLong(std::string_view value, std::string_view key) {
    const std::string s{value};

    std::size_t pos = 0;
    long result     = 0;

    try {
        result = std::stol(s, &pos);
    }
    catch (...) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to long", Here());
    }

    if (pos != s.size()) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to long without loss", Here());
    }

    return result;
}

inline double parsePlainDouble(std::string_view value, std::string_view key) {
    const std::string s{value};

    std::size_t pos = 0;
    double result   = 0.0;

    try {
        result = std::stod(s, &pos);
    }
    catch (...) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to double", Here());
    }

    if (pos != s.size()) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to double without loss", Here());
    }

    return result;
}

inline long parseHoursAsLong(std::string_view value, std::string_view key) {
    const std::string s{value};

    if (s.empty()) {
        throw exceptions::Mars2marsDictException("Cannot convert empty key `"s + std::string{key} + "` value to hours",
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
    // std::cout << "Parsing param value `" << value << "` as long for key `" << key << "`" << std::endl;
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

inline bool allDigits(std::string_view s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });
}

inline bool isLeapYear(long year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

inline long daysInMonth(long year, long month) {
    switch (month) {
        case 1:
            return 31;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        case 3:
            return 31;
        case 4:
            return 30;
        case 5:
            return 31;
        case 6:
            return 30;
        case 7:
            return 31;
        case 8:
            return 31;
        case 9:
            return 30;
        case 10:
            return 31;
        case 11:
            return 30;
        case 12:
            return 31;
        default:
            return 0;
    }
}

inline void checkYYYYMMDD(long yyyymmdd, std::string_view key) {
    const long year  = yyyymmdd / 10000;
    const long month = (yyyymmdd / 100) % 100;
    const long day   = yyyymmdd % 100;

    if (year <= 0 || month < 1 || month > 12 || day < 1 || day > daysInMonth(year, month)) {
        throw exceptions::Mars2marsDictException("Invalid MARS date `"s + std::to_string(yyyymmdd) + "` for key `"s +
                                                     std::string{key} + "`. Expected a valid yyyymmdd date.",
                                                 Here());
    }
}

inline long marsDateToLong(std::string_view value, std::string_view key) {
    const std::string s{value};

    std::string compact;

    if (s.size() == 8 && allDigits(s)) {
        compact = s;
    }
    else if (s.size() == 10 && s[4] == '-' && s[7] == '-' && allDigits(std::string_view{s.data(), 4}) &&
             allDigits(std::string_view{s.data() + 5, 2}) && allDigits(std::string_view{s.data() + 8, 2})) {
        compact.reserve(8);
        compact.append(s, 0, 4);
        compact.append(s, 5, 2);
        compact.append(s, 8, 2);
    }
    else {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s +
                "` to MARS date integer yyyymmdd. Supported forms are yyyymmdd and yyyy-mm-dd.",
            Here());
    }

    const long result = parsePlainLong(compact, key);
    checkYYYYMMDD(result, key);

    return result;
}

inline std::string longToMarsDate(long value) {
    const std::string s = std::to_string(value);

    if (s.size() != 8 || !allDigits(s)) {
        throw exceptions::Mars2marsDictException("Cannot convert integer `"s + s + "` to MARS date. Expected yyyymmdd.",
                                                 Here());
    }

    checkYYYYMMDD(value, "date");

    return s;
}

inline long parseTimePart(std::string_view part, std::string_view full, std::string_view key) {
    if (part.empty() || !allDigits(part)) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + std::string{full} + "` to MARS time.", Here());
    }

    return parsePlainLong(part, key);
}

inline long makeHHMMSS(long hh, long mm, long ss, std::string_view value, std::string_view key) {
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
        throw exceptions::Mars2marsDictException("Invalid MARS time `"s + std::string{value} + "` for key `"s +
                                                     std::string{key} + "`. Expected hh=[0,23], mm=[0,59], ss=[0,59].",
                                                 Here());
    }

    if (ss != 0) {
        throw exceptions::Mars2marsDictException("Invalid MARS time `"s + std::string{value} + "` for key `"s +
                                                     std::string{key} +
                                                     "`. Seconds are not supported by metkit MARS time normalisation.",
                                                 Here());
    }

    return hh * 10000 + mm * 100 + ss;
}

inline long parseSeparatedMarsTime(std::string_view value, std::string_view key, char sep) {

    const auto p0 = value.find(sep);

    if (p0 == std::string_view::npos) {
        throw exceptions::Mars2marsDictException(
            "Internal error while parsing separated MARS time `"s + std::string{value} + "`", Here());
    }

    const auto p1 = value.find(sep, p0 + 1);

    const auto hhPart = value.substr(0, p0);

    if (p1 == std::string_view::npos) {
        const auto mmPart = value.substr(p0 + 1);

        const long hh = parseTimePart(hhPart, value, key);
        const long mm = parseTimePart(mmPart, value, key);

        return makeHHMMSS(hh, mm, 0, value, key);
    }

    if (value.find(sep, p1 + 1) != std::string_view::npos) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + std::string{value} + "` to MARS time.", Here());
    }

    const auto mmPart = value.substr(p0 + 1, p1 - p0 - 1);
    const auto ssPart = value.substr(p1 + 1);

    const long hh = parseTimePart(hhPart, value, key);
    const long mm = parseTimePart(mmPart, value, key);
    const long ss = parseTimePart(ssPart, value, key);

    return makeHHMMSS(hh, mm, ss, value, key);
}

inline long marsTimeToLong(std::string_view value, std::string_view key) {
    const std::string s{value};

    if (s.empty()) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert empty key `"s + std::string{key} + "` value to MARS time.", Here());
    }

    if (s.find(':') != std::string::npos) {
        return parseSeparatedMarsTime(s, key, ':');
    }

    if (s.find('-') != std::string::npos) {
        return parseSeparatedMarsTime(s, key, '-');
    }

    if (!allDigits(s)) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to MARS time integer hhmmss.", Here());
    }

    if (s.size() <= 2) {
        // H or HH -> HH:00:00
        const long hh = parsePlainLong(s, key);
        return makeHHMMSS(hh, 0, 0, value, key);
    }

    if (s.size() <= 4) {
        // HMM or HHMM -> HH:MM:00
        std::string hhmm = s;
        hhmm.insert(hhmm.begin(), 4 - hhmm.size(), '0');

        const long hh = parsePlainLong(std::string_view{hhmm.data(), 2}, key);
        const long mm = parsePlainLong(std::string_view{hhmm.data() + 2, 2}, key);

        return makeHHMMSS(hh, mm, 0, value, key);
    }

    if (s.size() <= 6) {
        // HMMSS or HHMMSS -> HH:MM:SS
        std::string hhmmss = s;
        hhmmss.insert(hhmmss.begin(), 6 - hhmmss.size(), '0');

        const long hh = parsePlainLong(std::string_view{hhmmss.data(), 2}, key);
        const long mm = parsePlainLong(std::string_view{hhmmss.data() + 2, 2}, key);
        const long ss = parsePlainLong(std::string_view{hhmmss.data() + 4, 2}, key);

        return makeHHMMSS(hh, mm, ss, value, key);
    }

    throw exceptions::Mars2marsDictException(
        "Cannot convert key `"s + std::string{key} + "` value `"s + s + "` to MARS time integer hhmmss.", Here());
}

inline std::string longToMarsTime(long value) {
    if (value < 0 || value > 235959) {
        throw exceptions::Mars2marsDictException(
            "Cannot convert integer `"s + std::to_string(value) + "` to MARS time. Expected hhmmss.", Here());
    }

    std::ostringstream os;
    os << std::setw(6) << std::setfill('0') << value;

    const std::string hhmmss = os.str();

    const long hh = parsePlainLong(std::string_view{hhmmss.data(), 2}, "time");
    const long mm = parsePlainLong(std::string_view{hhmmss.data() + 2, 2}, "time");
    const long ss = parsePlainLong(std::string_view{hhmmss.data() + 4, 2}, "time");

    makeHHMMSS(hh, mm, ss, hhmmss, "time");

    // metkit canonical MARS time is HHMM, not HHMMSS.
    return hhmmss.substr(0, 4);
}

inline const std::unordered_map<std::string, ToLong>& toLongConverters() {
    static const std::unordered_map<std::string, ToLong> converters = {
        {"param", paramToLong},    {"levelist", plainLong},  {"chem", plainLong},
        {"step", hoursLong},       {"timespan", hoursLong},  {"date", marsDateToLong},
        {"hdate", marsDateToLong}, {"time", marsTimeToLong}, {"htime", marsTimeToLong},
    };

    return converters;
}

inline const std::unordered_map<std::string, FromLong>& fromLongConverters() {
    static const std::unordered_map<std::string, FromLong> converters = {
        {"param", plainLongString}, {"levelist", plainLongString}, {"chem", plainLongString},
        {"step", longToHours},      {"timespan", longToHours},     {"date", longToMarsDate},
        {"hdate", longToMarsDate},  {"time", longToMarsTime},      {"htime", longToMarsTime},
    };

    return converters;
}

inline long convertToLongOrThrow(std::string_view key, std::string_view value) {

    const std::string k{key};

    const auto& converters = toLongConverters();
    const auto it          = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support conversion to long for metkit::mars::MarsRequest", Here());
    }

    return it->second(value, key);
}

inline std::string convertFromLongOrThrow(std::string_view key, long value) {

    const std::string k{key};

    const auto& converters = fromLongConverters();
    const auto it          = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support assignment from long for metkit::mars::MarsRequest", Here());
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

inline double convertToDoubleOrThrow(std::string_view key, std::string_view value) {

    const std::string k{key};

    const auto& converters = toDoubleConverters();
    const auto it          = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support conversion to double for metkit::mars::MarsRequest", Here());
    }

    return it->second(value, key);
}

inline std::string convertFromDoubleOrThrow(std::string_view key, double value) {

    const std::string k{key};

    const auto& converters = fromDoubleConverters();
    const auto it          = converters.find(k);

    if (it == converters.end()) {
        throw exceptions::Mars2marsDictException(
            "Key `"s + k + "` does not support assignment from double for metkit::mars::MarsRequest", Here());
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

    static std::unique_ptr<metkit::mars::MarsRequest> clone_or_throw(const metkit::mars::MarsRequest& mars) {
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
    static std::string get_or_throw(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {

        return detail::scalarStringOrThrow(mars, key);
    }
};

template <>
struct DictGetOpt<metkit::mars::MarsRequest, std::string> {
    static std::optional<std::string> get_opt(const metkit::mars::MarsRequest& mars,
                                              std::string_view key) noexcept(false) {

        return detail::scalarStringOpt(mars, key);
    }
};

template <>
struct DictSetOrThrow<metkit::mars::MarsRequest, std::string> {
    static void set_or_throw(metkit::mars::MarsRequest& mars, std::string_view key,
                             const std::string& value) noexcept(false) {

        mars.setValue(std::string{key}, value);
    }
};

template <>
struct DictSetOrIgnore<metkit::mars::MarsRequest, std::string> {
    static void set_or_ignore(metkit::mars::MarsRequest& mars, std::string_view key,
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
    static long get_or_throw(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {

        const auto& raw = detail::scalarStringOrThrow(mars, key);
        return detail::convertToLongOrThrow(key, raw);
    }
};

template <>
struct DictGetOpt<metkit::mars::MarsRequest, long> {
    static std::optional<long> get_opt(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {

        const auto raw = detail::scalarStringOpt(mars, key);

        if (!raw.has_value()) {
            return std::nullopt;
        }

        return detail::convertToLongOrThrow(key, *raw);
    }
};

template <>
struct DictSetOrThrow<metkit::mars::MarsRequest, long> {
    static void set_or_throw(metkit::mars::MarsRequest& mars, std::string_view key, const long& value) noexcept(false) {

        mars.setValue(std::string{key}, detail::convertFromLongOrThrow(key, value));
    }
};

template <>
struct DictSetOrIgnore<metkit::mars::MarsRequest, long> {
    static void set_or_ignore(metkit::mars::MarsRequest& mars, std::string_view key,
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
    static double get_or_throw(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {

        const auto& raw = detail::scalarStringOrThrow(mars, key);
        return detail::convertToDoubleOrThrow(key, raw);
    }
};

template <>
struct DictGetOpt<metkit::mars::MarsRequest, double> {
    static std::optional<double> get_opt(const metkit::mars::MarsRequest& mars, std::string_view key) noexcept(false) {

        const auto raw = detail::scalarStringOpt(mars, key);

        if (!raw.has_value()) {
            return std::nullopt;
        }

        return detail::convertToDoubleOrThrow(key, *raw);
    }
};

template <>
struct DictSetOrThrow<metkit::mars::MarsRequest, double> {
    static void set_or_throw(metkit::mars::MarsRequest& mars, std::string_view key,
                             const double& value) noexcept(false) {

        mars.setValue(std::string{key}, detail::convertFromDoubleOrThrow(key, value));
    }
};

template <>
struct DictSetOrIgnore<metkit::mars::MarsRequest, double> {
    static void set_or_ignore(metkit::mars::MarsRequest& mars, std::string_view key,
                              const double& value) noexcept(false) {

        try {
            mars.setValue(std::string{key}, detail::convertFromDoubleOrThrow(key, value));
        }
        catch (...) {
        }
    }
};

}  // namespace metkit::mars2mars::utils::dict_traits
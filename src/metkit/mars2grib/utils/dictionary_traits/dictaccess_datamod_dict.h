#pragma once

/// @file dictaccess_datamod_dict.h
/// @brief Dictionary access traits for metkit::datamod::Dict.
///
/// Reproduces the behaviour of the eckit::LocalConfiguration traits (dictaccess_eckit_configuration.h)
/// type by type, so the encoder behaves the same with either dictionary:
///   - `has` is true for any present key, also when its value is null;
///   - integers are integral; integers and doubles are accepted as floating point;
///   - lists are typed; an empty list counts as a list of any type (as in eckit::Configuration);
///   - null matches no type: get_opt returns nullopt, get_or_throw throws.

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/datamod/Dict.h"
#include "metkit/datamod/Json.h"
#include "metkit/mars2grib/utils/generalUtils.h"

#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/type_traits_name.h"

namespace metkit::mars2grib::utils {

template <>
constexpr std::string_view type_name<metkit::datamod::Dict>() {
    return "metkit::datamod::Dict";
}

template <>
constexpr std::string_view type_name<std::vector<metkit::datamod::Dict>>() {
    return "vector<metkit::datamod::Dict>";
}

}  // namespace metkit::mars2grib::utils

namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

namespace datamod_access {

using metkit::datamod::Dict;
using metkit::datamod::Value;

inline bool isEmptyList(const Value& v) {
    return (v.isIntList() || v.isDoubleList() || v.isStringList() || v.isDictList()) && v.listSize() == 0;
}

/// Conversions between a stored Value and the C++ type requested by the encoder.
template <typename T>
struct Access;

template <>
struct Access<bool> {
    static bool matches(const Value& v) { return v.isBool(); }
    static bool get(const Value& v) { return v.asBool(); }
};

template <>
struct Access<int> {
    static bool matches(const Value& v) { return v.isInt(); }
    static int get(const Value& v) { return static_cast<int>(v.asInt()); }
};

template <>
struct Access<long> {
    static bool matches(const Value& v) { return v.isInt(); }
    static long get(const Value& v) { return static_cast<long>(v.asInt()); }
};

template <>
struct Access<double> {
    static bool matches(const Value& v) { return v.isDouble() || v.isInt(); }
    static double get(const Value& v) { return v.isInt() ? static_cast<double>(v.asInt()) : v.asDouble(); }
};

template <>
struct Access<float> {
    static bool matches(const Value& v) { return Access<double>::matches(v); }
    static float get(const Value& v) { return static_cast<float>(Access<double>::get(v)); }
};

template <>
struct Access<std::string> {
    static bool matches(const Value& v) { return v.isString(); }
    static std::string get(const Value& v) { return v.asString(); }
};

template <>
struct Access<Dict> {
    static bool matches(const Value& v) { return v.isDict(); }
    static Dict get(const Value& v) { return v.asDict(); }
};

template <typename Int>
struct IntListAccess {
    static bool matches(const Value& v) { return v.isIntList() || isEmptyList(v); }
    static std::vector<Int> get(const Value& v) {
        if (!v.isIntList()) {
            return {};
        }
        const auto& list = v.asIntList();
        return std::vector<Int>(list.begin(), list.end());
    }
};

template <>
struct Access<std::vector<int>> : IntListAccess<int> {};

template <>
struct Access<std::vector<long>> : IntListAccess<long> {};

template <typename Real>
struct RealListAccess {
    static bool matches(const Value& v) { return v.isDoubleList() || v.isIntList() || isEmptyList(v); }
    static std::vector<Real> get(const Value& v) {
        if (v.isDoubleList()) {
            const auto& list = v.asDoubleList();
            return std::vector<Real>(list.begin(), list.end());
        }
        if (v.isIntList()) {
            const auto& list = v.asIntList();
            return std::vector<Real>(list.begin(), list.end());
        }
        return {};
    }
};

template <>
struct Access<std::vector<double>> : RealListAccess<double> {};

template <>
struct Access<std::vector<float>> : RealListAccess<float> {};

template <>
struct Access<std::vector<std::string>> {
    static bool matches(const Value& v) { return v.isStringList() || isEmptyList(v); }
    static std::vector<std::string> get(const Value& v) {
        return v.isStringList() ? v.asStringList() : std::vector<std::string>{};
    }
};

template <>
struct Access<std::vector<Dict>> {
    static bool matches(const Value& v) { return v.isDictList() || isEmptyList(v); }
    static std::vector<Dict> get(const Value& v) { return v.isDictList() ? v.asDictList() : std::vector<Dict>{}; }
};

}  // namespace datamod_access

// -----------------------------------------------------------------------------
// to_json
// -----------------------------------------------------------------------------
template <>
struct DictToJsonTraits<metkit::datamod::Dict> {
    static std::string to_json(const metkit::datamod::Dict& dict) noexcept(true) {
        try {
            return metkit::datamod::toJson(dict);
        }
        catch (...) {
            return "[to_json failed for metkit::datamod::Dict]";
        }
    }
};

// -----------------------------------------------------------------------------
// make_from_sample / clone / checks
// -----------------------------------------------------------------------------
template <>
struct DictTraits<metkit::datamod::Dict> {
    static constexpr bool support_checks = false;

    static std::unique_ptr<metkit::datamod::Dict> make_from_sample_or_throw(std::string_view name) {
        auto dict = std::make_unique<metkit::datamod::Dict>();
        dict->set("SampleName", metkit::datamod::Value{std::string(name)});
        return dict;
    }

    static std::unique_ptr<metkit::datamod::Dict> clone_or_throw(const metkit::datamod::Dict& dict) {
        return std::make_unique<metkit::datamod::Dict>(dict);
    }
};

// -----------------------------------------------------------------------------
// has
// -----------------------------------------------------------------------------
template <>
struct DictHas<metkit::datamod::Dict> {
    static bool has(const metkit::datamod::Dict& dict, std::string_view key) noexcept(false) { return dict.has(key); }
};

// -----------------------------------------------------------------------------
// get / set for every type supported by the LocalConfiguration traits
// -----------------------------------------------------------------------------
template <class T>
struct DictGetOrThrow<metkit::datamod::Dict, T> {
    static T get_or_throw(const metkit::datamod::Dict& dict, std::string_view key) noexcept(false) {
        using Access = datamod_access::Access<T>;
        try {
            const metkit::datamod::Value* value = dict.find(key);
            if (value == nullptr) {
                throw exceptions::Mars2GribDictException("Missing key `"s + std::string(key) +
                                                             "` in dictionary type `"s +
                                                             std::string(type_name<metkit::datamod::Dict>()) + "`",
                                                         Here());
            }
            if (!Access::matches(*value)) {
                throw exceptions::Mars2GribDictException("Key `"s + std::string(key) + "` is not of expected type `"s +
                                                             std::string(type_name<T>()) + "` for dictionary type `"s +
                                                             std::string(type_name<metkit::datamod::Dict>()) + "`",
                                                         Here());
            }
            return Access::get(*value);
        }
        catch (const exceptions::Mars2GribGenericException&) {
            throw;
        }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException(
                "Internal error while reading key `"s + std::string(key) + "` as `" + std::string(type_name<T>()) +
                    "` from dictionary type `"s + std::string(type_name<metkit::datamod::Dict>()) + "`",
                Here()));
        }
        mars2gribUnreachable();
    }
};

template <class T>
struct DictGetOpt<metkit::datamod::Dict, T> {
    static std::optional<T> get_opt(const metkit::datamod::Dict& dict, std::string_view key) noexcept(false) {
        using Access = datamod_access::Access<T>;
        try {
            const metkit::datamod::Value* value = dict.find(key);
            if (value == nullptr || !Access::matches(*value)) {
                return std::nullopt;
            }
            return Access::get(*value);
        }
        catch (...) {
            return std::nullopt;
        }
        mars2gribUnreachable();
    }
};

template <class T>
struct DictSetOrThrow<metkit::datamod::Dict, T> {
    static void set_or_throw(metkit::datamod::Dict& dict, std::string_view key, const T& value) noexcept(false) {
        try {
            dict.set(key, metkit::datamod::Value{value});
            return;
        }
        catch (const exceptions::Mars2GribGenericException&) {
            throw;
        }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException(
                "Unable to set key `"s + std::string(key) + "` with type `"s + std::string(type_name<T>()) +
                    "` in dictionary type `"s + std::string(type_name<metkit::datamod::Dict>()) + "`",
                Here()));
        }
        mars2gribUnreachable();
    }
};

template <class T>
struct DictSetOrIgnore<metkit::datamod::Dict, T> {
    static void set_or_ignore(metkit::datamod::Dict& dict, std::string_view key, const T& value) noexcept(false) {
        try {
            dict.set(key, metkit::datamod::Value{value});
        }
        catch (...) {
        }
    }
};

}  // namespace metkit::mars2grib::utils::dict_traits

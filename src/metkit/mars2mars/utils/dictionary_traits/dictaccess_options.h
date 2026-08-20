/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file dictaccess_options.h
/// @brief Read-only dictionary-access traits for Mars2Mars Options.
///
/// This adapter exposes the strongly typed `Options` aggregate through the
/// generic Mars2Mars dictionary API.
///
/// Supported operations for the bool-backed and string-backed option keys:
///
/// - `has(options, key)`
/// - `has<bool>(options, key)`
/// - `get_opt<bool>(options, key)`
/// - `get_or_throw<bool>(options, key)`
/// - `dict_to_json(options)`
///
/// Deliberately unsupported operations:
///
/// - mutation through dictionary traits;
/// - cloning through dictionary traits;
/// - construction from a sample;
/// - typed access beyond the explicitly supported bool/string keys.
///
/// The Options object is already a complete policy snapshot. The adapter is
/// therefore read-only.
///
#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "metkit/mars2mars/api/Options.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/type_traits_name.h"

namespace metkit::mars2mars::utils {

///
/// @brief Human-readable type name for the Options dictionary.
///
template <>
constexpr std::string_view type_name<metkit::mars2mars::Options>() {
    return "metkit::mars2mars::Options";
}

}  // namespace metkit::mars2mars::utils

namespace metkit::mars2mars::utils::dict_traits {

namespace options_detail {

inline void appendJsonBool(std::string& json, std::string_view key, bool value, bool trailingComma) {

    json += "\"";
    json += key;
    json += "\":";
    json += value ? "true" : "false";

    if (trailingComma) {
        json += ',';
    }
}

inline void appendJsonString(std::string& json, std::string_view key, std::string_view value, bool trailingComma) {

    json += '"';
    json += key;
    json += "\":\"";

    for (const char c : value) {
        if (c == '\\' || c == '"') {
            json += '\\';
        }
        json += c;
    }

    json += '"';

    if (trailingComma) {
        json += ',';
    }
}

inline bool isBoolKey(std::string_view key) noexcept {
    return key == "saveErrorStack" || key == "printErrorStackToStdErr" || key == "skipSection3" ||
           key == "tryFixBadInput_ZeroAccumulation";
}

inline bool isStringKey(std::string_view key) noexcept {
    return key == "errorStackPath";
}

inline bool isKnownKey(std::string_view key) noexcept {
    return isBoolKey(key) || isStringKey(key);
}

inline bool getBoolOrThrow(const Options& opts, std::string_view key) {

    if (key == "saveErrorStack") {
        return opts.saveErrorStack;
    }

    if (key == "printErrorStackToStdErr") {
        return opts.printErrorStackToStdErr;
    }

    if (key == "skipSection3") {
        return opts.skipSection3;
    }

    if (key == "tryFixBadInput_ZeroAccumulation") {
        return opts.tryFixBadInput_ZeroAccumulation;
    }

    throw exceptions::Mars2marsDictException("Key `" + std::string(key) + "` cannot be read as `" + "bool" +
                                                 "` from dictionary type `metkit::mars2mars::Options`",
                                             Here());
}

inline std::string getStringOrThrow(const Options& opts, std::string_view key) {

    if (key == "errorStackPath") {
        return opts.errorStackPath;
    }

    throw exceptions::Mars2marsDictException("Key `" + std::string(key) + "` cannot be read as `" + "std::string" +
                                                 "` from dictionary type `metkit::mars2mars::Options`",
                                             Here());
}

}  // namespace options_detail

template <>
struct DictToJsonTraits<Options> {

    static std::string to_json(const Options& opts) noexcept(true) {
        try {
            std::string json;
            json.reserve(128);
            json += '{';
            options_detail::appendJsonBool(json, "saveErrorStack", opts.saveErrorStack, true);
            options_detail::appendJsonBool(json, "skipSection3", opts.skipSection3, true);
            options_detail::appendJsonBool(json, "tryFixBadInput_ZeroAccumulation",
                                           opts.tryFixBadInput_ZeroAccumulation, true);
            options_detail::appendJsonString(json, "errorStackPath", opts.errorStackPath, true);
            options_detail::appendJsonBool(json, "printErrorStackToStdErr", opts.printErrorStackToStdErr, false);
            json += '}';
            return json;
        }
        catch (...) {
            return "[to_json failed for metkit::mars2mars::Options]";
        }
    }
};

template <>
struct DictHas<Options> {

    static bool has(const Options&, std::string_view key) noexcept(false) {

        if (options_detail::isKnownKey(key)) {
            return true;
        }

        return false;
    }
};

template <>
struct DictGetOrThrow<Options, bool> {

    static bool get_or_throw(const Options& opts, std::string_view key) noexcept(false) {

        return options_detail::getBoolOrThrow(opts, key);
    }
};

template <>
struct DictGetOpt<Options, bool> {

    static std::optional<bool> get_opt(const Options& opts, std::string_view key) noexcept(false) {

        if (!options_detail::isBoolKey(key)) {
            return std::nullopt;
        }

        return options_detail::getBoolOrThrow(opts, key);
    }
};

template <>
struct DictGetOrThrow<Options, std::string> {

    static std::string get_or_throw(const Options& opts, std::string_view key) noexcept(false) {

        return options_detail::getStringOrThrow(opts, key);
    }
};

template <>
struct DictGetOpt<Options, std::string> {

    static std::optional<std::string> get_opt(const Options& opts, std::string_view key) noexcept(false) {

        if (!options_detail::isStringKey(key)) {
            return std::nullopt;
        }

        return options_detail::getStringOrThrow(opts, key);
    }
};

/*
 * No explicit function-template specialisation is required for `has<T>()`.
 *
 * The generic dictionary API implements:
 *
 *   has<T>(options, key)
 *
 * by invoking:
 *
 *   DictGetOpt<Options, T>::get_opt(options, key).has_value()
 *
 * Therefore the DictGetOpt specialisation above provides:
 *
 * - has<bool>(options, key)
 * - has<std::string>(options, key)
 */
}  // namespace metkit::mars2mars::utils::dict_traits

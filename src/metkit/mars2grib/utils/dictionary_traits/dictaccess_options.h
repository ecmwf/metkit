/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file dictaccess_options.h
/// @brief Read-only dictionary-access traits for Mars2Grib Options.
///
/// This adapter exposes the strongly typed `Options` aggregate through the
/// generic Mars2Grib dictionary API.
///
/// Supported operations for the bool-backed option keys:
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
/// - non-bool typed access through dictionary traits.
///
/// The Options object is already a complete policy snapshot. The adapter is
/// therefore read-only.
///
#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/type_traits_name.h"

namespace metkit::mars2grib::utils {

///
/// @brief Human-readable type name used by generic dictionary diagnostics.
///
template <>
constexpr std::string_view type_name<metkit::mars2grib::backend::tables::TypeOfTimeIntervals>() {
    return "TypeOfTimeIntervals";
}

///
/// @brief Human-readable type name for the Options dictionary.
///
template <>
constexpr std::string_view type_name<metkit::mars2grib::Options>() {
    return "metkit::mars2grib::Options";
}

}  // namespace metkit::mars2grib::utils

namespace metkit::mars2grib::utils::dict_traits {

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

inline bool isBoolKey(std::string_view key) noexcept {
    return key == "applyChecks" || key == "enableOverride" || key == "enableBitsPerValueCompression" ||
           key == "normalizeMars" || key == "normalizeMisc" || key == "fixMarsGrid" || key == "skipSection3" ||
           key == "allowDefaultTimeIncrement" || key == "allowZeroLengthFsWindow" ||
           key == "allowExtendedSetOfOperationsForZeroLengthFsWindow" ||
           key == "allowNonEnumeratedPositiveIntegerTimespanHours" || key == "allowRedundantTimeIncrement" ||
           key == "allowMissingTimespanForInstantProduct" || key == "allowMissingTimespanForStatisticalProduct";
}

inline bool isKnownKey(std::string_view key) noexcept {
    return isBoolKey(key);
}


[[noreturn]] inline void throwMissingOptionalValue(std::string_view key, std::string_view requestedType) {

    throw exceptions::Mars2GribDictException(
        "Key `" + std::string(key) + "` has no materialised value while reading `" + std::string(requestedType) +
            "` from dictionary type `metkit::mars2grib::Options`",
        Here());
}

inline bool getBoolOrThrow(const Options& opts, std::string_view key) {

    if (key == "applyChecks") {
        return opts.applyChecks;
    }

    if (key == "enableOverride") {
        return opts.enableOverride;
    }

    if (key == "enableBitsPerValueCompression") {
        return opts.enableBitsPerValueCompression;
    }

    if (key == "normalizeMars") {
        return opts.normalizeMars;
    }

    if (key == "normalizeMisc") {
        return opts.normalizeMisc;
    }

    if (key == "fixMarsGrid") {
        return opts.fixMarsGrid;
    }

    if (key == "skipSection3") {
        return opts.skipSection3;
    }

    if (key == "allowDefaultTimeIncrement") {
        return opts.allowDefaultTimeIncrement;
    }

    if (key == "allowZeroLengthFsWindow") {
        return opts.allowZeroLengthFsWindow;
    }

    if (key == "allowExtendedSetOfOperationsForZeroLengthFsWindow") {
        return opts.allowExtendedSetOfOperationsForZeroLengthFsWindow;
    }

    if (key == "allowNonEnumeratedPositiveIntegerTimespanHours") {
        return opts.allowNonEnumeratedPositiveIntegerTimespanHours;
    }

    if (key == "allowRedundantTimeIncrement") {
        return opts.allowRedundantTimeIncrement;
    }

    if (key == "allowMissingTimespanForInstantProduct") {
        return opts.allowMissingTimespanForInstantProduct;
    }

    if (key == "allowMissingTimespanForStatisticalProduct") {
        return opts.allowMissingTimespanForStatisticalProduct;
    }

    throw exceptions::Mars2GribDictException("Key `" + std::string(key) + "` cannot be read as `" + "bool" +
                                                 "` from dictionary type `metkit::mars2grib::Options`",
                                             Here());
}

}  // namespace options_detail

// -----------------------------------------------------------------------------
// to_json
// -----------------------------------------------------------------------------

template <>
struct DictToJsonTraits<Options> {

    static std::string to_json(const Options& opts) noexcept(true) {
        try {
            std::string json;
            json.reserve(512);
            json += '{';
            options_detail::appendJsonBool(json, "applyChecks", opts.applyChecks, true);
            options_detail::appendJsonBool(json, "enableOverride", opts.enableOverride, true);
            options_detail::appendJsonBool(json, "enableBitsPerValueCompression", opts.enableBitsPerValueCompression,
                                           true);
            options_detail::appendJsonBool(json, "normalizeMars", opts.normalizeMars, true);
            options_detail::appendJsonBool(json, "normalizeMisc", opts.normalizeMisc, true);
            options_detail::appendJsonBool(json, "fixMarsGrid", opts.fixMarsGrid, true);
            options_detail::appendJsonBool(json, "skipSection3", opts.skipSection3, true);
            options_detail::appendJsonBool(json, "allowDefaultTimeIncrement", opts.allowDefaultTimeIncrement, true);
            options_detail::appendJsonBool(json, "allowZeroLengthFsWindow", opts.allowZeroLengthFsWindow, true);
            options_detail::appendJsonBool(json, "allowExtendedSetOfOperationsForZeroLengthFsWindow",
                                           opts.allowExtendedSetOfOperationsForZeroLengthFsWindow, true);
            options_detail::appendJsonBool(json, "allowNonEnumeratedPositiveIntegerTimespanHours",
                                           opts.allowNonEnumeratedPositiveIntegerTimespanHours, true);
            options_detail::appendJsonBool(json, "allowRedundantTimeIncrement", opts.allowRedundantTimeIncrement, true);
            options_detail::appendJsonBool(json, "allowMissingTimespanForInstantProduct",
                                           opts.allowMissingTimespanForInstantProduct, true);
            options_detail::appendJsonBool(json, "allowMissingTimespanForStatisticalProduct",
                                           opts.allowMissingTimespanForStatisticalProduct, false);
            json += '}';
            return json;
        }
        catch (...) {
            return "[to_json failed for metkit::mars2grib::Options]";
        }
    }
};

// -----------------------------------------------------------------------------
// has(options, key)
// -----------------------------------------------------------------------------

///
/// @brief Report whether an option has a readable materialised value.
///
/// Every supported Options key is a non-optional bool member and is therefore
/// always present. Unknown keys are absent.
///
/// This definition gives the untyped `has(options, key)` operation normal
/// dictionary semantics rather than merely reporting whether the C++ structure
/// declares a member with that name.
///
template <>
struct DictHas<Options> {

    static bool has(const Options& opts, std::string_view key) noexcept(false) {

        if (options_detail::isBoolKey(key)) {
            return true;
        }

        return false;
    }
};

// -----------------------------------------------------------------------------
// bool access
// -----------------------------------------------------------------------------

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
 */
}  // namespace metkit::mars2grib::utils::dict_traits

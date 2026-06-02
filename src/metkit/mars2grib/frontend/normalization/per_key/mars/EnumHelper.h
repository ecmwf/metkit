/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file EnumHelper.h
/// @brief Shared strict alias-to-canonical resolution for enum-typed MARS keys.
///
/// The helper consults the MARS language definition (loaded lazily from
/// the metkit @c language.yaml on first use, and cached process-wide) to
/// resolve a user-supplied value to its canonical form. Each enum row in
/// the YAML, e.g.
///
/// @code
/// origin:
///   values:
///     - [ecmf, 98, ecmwf]
/// @endcode
///
/// declares @c ecmf as canonical and @c 98 / @c ecmwf as exact synonyms.
/// Any of the three is accepted; the helper returns @c "ecmf" in all cases.
///
/// @note Resolution is strict equality: case-variants and prefix matches
/// are rejected. The caller is expected to supply a valid value or fix
/// their input.
///
#pragma once

// System includes
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>

// Project includes
#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend::normalization::per_key::enum_helper {

namespace detail {

///
/// @brief Lazily-loaded MARS language definition for the @c retrieve verb.
///
/// Returned reference is process-static and initialized on first call,
/// matching the lifetime model used by metkit::mars::MarsLanguage.
///
inline const eckit::Value& retrieve_language() {
    static const eckit::Value language = [] {
        const eckit::Value all = eckit::YAMLParser::decodeFile(metkit::LibMetkit::languageYamlFile());
        return all["retrieve"];
    }();
    return language;
}

///
/// @brief Stringify an eckit::Value scalar for alias comparison.
///
/// YAML rows mix bare integers (e.g. @c 98) and bare strings (e.g. @c ecmf).
/// We compare against either by stringifying both sides.
///
inline std::string scalar_to_string(const eckit::Value& v) {
    if (v.isString()) {
        return static_cast<std::string>(v);
    }
    if (v.isNumber()) {
        return std::to_string(static_cast<long long>(v));
    }
    if (v.isDouble()) {
        std::ostringstream oss;
        oss << static_cast<double>(v);
        return oss.str();
    }
    return static_cast<std::string>(v);
}

///
/// @brief Per-key enum sub-tree from the language definition.
///
/// Returns a nil value if @p key has no entry or carries no flat
/// @c values list (e.g. context-conditioned types like @c dataset).
///
inline eckit::Value enum_values(const eckit::Value& language, std::string_view key) {
    if (language.isNil() || !language.isMap() || !language.contains(std::string{key})) {
        return eckit::Value{};
    }
    const eckit::Value def = language[std::string{key}];
    if (!def.isMap() || !def.contains("values")) {
        return eckit::Value{};
    }
    const eckit::Value values = def["values"];
    return values.isList() ? values : eckit::Value{};
}

///
/// @brief Build a one-line listing of allowed canonical values for diagnostics.
///
inline std::string allowed_summary(const eckit::Value& values) {
    std::ostringstream oss;
    for (size_t i = 0; i < values.size(); ++i) {
        const eckit::Value row = values[i];
        if (i > 0)
            oss << ", ";
        if (row.isList() && row.size() > 0) {
            oss << scalar_to_string(row[0]);
        }
        else {
            oss << scalar_to_string(row);
        }
    }
    return oss.str();
}

}  // namespace detail


///
/// @brief Resolve @p value to its canonical string against @p language[@p key].
///
/// @param language MARS language tree. If nil, the lazily-loaded
///                 @c retrieve language from @c language.yaml is used.
/// @param key      MARS key name (e.g. @c "origin").
/// @param value    User-supplied alias (canonical, synonym, or numeric synonym
///                 stringified).
///
/// @return The canonical string (first element of the matched row).
///
/// @throws Mars2GribGenericException if @p value is not present in any
/// alias row of the enum definition.
///
inline std::string resolve_canonical(const eckit::Value& language, std::string_view key,
                                     const std::string& value) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    const eckit::Value& lang = language.isNil() ? detail::retrieve_language() : language;
    const eckit::Value values = detail::enum_values(lang, key);

    if (values.isNil()) {
        // No flat enum definition available for this key. Pass through to
        // preserve forward compatibility with context-conditioned types.
        return value;
    }

    for (size_t i = 0; i < values.size(); ++i) {
        const eckit::Value row = values[i];
        if (!row.isList() || row.size() == 0) {
            continue;
        }
        for (size_t j = 0; j < row.size(); ++j) {
            if (detail::scalar_to_string(row[j]) == value) {
                return detail::scalar_to_string(row[0]);
            }
        }
    }

    std::ostringstream msg;
    msg << "Invalid value for MARS key '" << key << "': '" << value
        << "'. Allowed canonical values: [" << detail::allowed_summary(values) << "].";
    throw Mars2GribGenericException(msg.str(), Here());
}

///
/// @brief Integer overload — looks up @p value stringified.
///
inline std::string resolve_canonical(const eckit::Value& language, std::string_view key, long value) {
    return resolve_canonical(language, key, std::to_string(value));
}

///
/// @brief Resolve @p value against a key that may carry both enum aliases and
///        regex patterns (e.g. @c grid).
///
/// Resolution order:
/// 1. Flat enum alias lookup (same as @ref resolve_canonical).
/// 2. If no alias hit: iterate @c language[key]["regex"] list and test each
///    pattern via @c std::regex_match.
/// 3. On a regex match: if @c language[key]["uppercase"] is @c true, return
///    the value converted to uppercase; otherwise return as-is.
/// 4. If neither alias nor regex matches: throw.
/// 5. If @p language is nil or the key carries no @c values/@c regex entry:
///    lenient pass-through (returns @p value unchanged).
///
inline std::string resolve_canonical_or_regex(const eckit::Value& language, std::string_view key,
                                              const std::string& value) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    const eckit::Value& lang = language.isNil() ? detail::retrieve_language() : language;

    // --- Step 1: enum alias lookup ----------------------------------------
    const eckit::Value values = detail::enum_values(lang, key);
    if (!values.isNil()) {
        for (size_t i = 0; i < values.size(); ++i) {
            const eckit::Value row = values[i];
            if (!row.isList() || row.size() == 0) {
                // Bare scalar row (e.g. "auto")
                if (detail::scalar_to_string(row) == value) {
                    return value;
                }
                continue;
            }
            for (size_t j = 0; j < row.size(); ++j) {
                if (detail::scalar_to_string(row[j]) == value) {
                    return detail::scalar_to_string(row[0]);
                }
            }
        }
    }

    // --- Step 2: regex patterns -------------------------------------------
    const eckit::Value keyDef =
        (lang.isMap() && lang.contains(std::string{key})) ? lang[std::string{key}] : eckit::Value{};
    if (!keyDef.isNil() && keyDef.isMap() && keyDef.contains("regex")) {
        const eckit::Value patterns = keyDef["regex"];
        if (patterns.isList()) {
            for (size_t i = 0; i < patterns.size(); ++i) {
                const std::string pat = detail::scalar_to_string(patterns[i]);
                try {
                    if (std::regex_match(value, std::regex{pat})) {
                        // --- Step 3: apply uppercase if required ----------
                        bool doUppercase = false;
                        if (keyDef.contains("uppercase")) {
                            try {
                                doUppercase = static_cast<bool>(keyDef["uppercase"]);
                            }
                            catch (...) {
                            }
                        }
                        if (doUppercase) {
                            std::string up = value;
                            std::transform(up.begin(), up.end(), up.begin(),
                                           [](unsigned char c) { return std::toupper(c); });
                            return up;
                        }
                        return value;
                    }
                }
                catch (const std::regex_error&) {
                    // Malformed pattern in YAML — skip silently.
                }
            }
        }
    }

    // --- Step 4: nothing matched -------------------------------------------
    // If there was no definition at all, pass through leniently (Step 5).
    if (values.isNil() && (keyDef.isNil() || !keyDef.contains("regex"))) {
        return value;
    }

    std::ostringstream msg;
    msg << "Invalid value for MARS key '" << key << "': '" << value << "'.";
    if (!values.isNil()) {
        msg << " Allowed enum values: [" << detail::allowed_summary(values) << "].";
    }
    throw Mars2GribGenericException(msg.str(), Here());
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key::enum_helper

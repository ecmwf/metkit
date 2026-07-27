/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file dictionary_access_traits.h
/// @brief Generic dictionary access traits for grib2mars.
#pragma once

#include <cxxabi.h>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "eckit/exception/Exceptions.h"
#include "metkit/grib2mars/utils/generalUtils.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"
#include "metkit/grib2mars/utils/type_traits_name.h"


namespace metkit::grib2mars::utils::dict_traits {

using std::operator""s;

template <typename>
struct dependent_false : std::false_type {};


/// @brief Trait hook used to render a dictionary as JSON.
template <typename Dict>
struct DictToJsonTraits {

    static std::string to_json(const Dict&) { return std::string{"[to_json not supported for this dictionary type]"}; }

    static void dump_or_ignore(const Dict&, const std::string&) {
        LOG_DEBUG_LIB(LibMetkit) << to_json(std::declval<Dict>());
    }
};

/// @brief Trait hook for cloning and sample creation.
template <typename Dict>
struct DictTraits {
    static constexpr bool support_checks = false;

    static std::unique_ptr<Dict> make_from_sample_or_throw(std::string_view) {
        static_assert(dependent_false<Dict>::value, "DictTraits::make_from_sample_or_throw not specialized");
    }

    static std::unique_ptr<Dict> clone_or_throw(const Dict&) {
        static_assert(dependent_false<Dict>::value, "DictTraits::clone_or_throw not specialized");
    }
};

/// @brief Trait hook for key presence checks.
template <class Dict>
struct DictHas {

    static bool has(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictHas not specialized for this Dict");
        grib2marsUnreachable();
    }
};


/// @brief Trait hook for missing-value checks.
template <class Dict>
struct DictMissing {

    static bool isMissing(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictMissing not specialized for this Dict");
        grib2marsUnreachable();
    }

    static void setMissing(Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictMissing not specialized for this Dict");
        grib2marsUnreachable();
    }
};

/// @brief Trait hook for optional value retrieval.
template <class Dict, class T>
struct DictGetOpt {

    static std::optional<T> get_opt(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictGetOpt not specialized for this Dict and type");
        grib2marsUnreachable();
    }
};

/// @brief Trait hook for mandatory value retrieval.
template <class Dict, class T>
struct DictGetOrThrow {

    static T get_or_throw(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictGetOrThrow not specialized for this Dict and type");
        grib2marsUnreachable();
    }
};

/// @brief Trait hook for assignment that may be ignored.
template <class Dict, class T>
struct DictSetOrIgnore {
    static void set_or_ignore(Dict&, std::string_view, const T&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictSetOrIgnore not specialized for this Dict and type");
        grib2marsUnreachable();
    }
};


/// @brief Trait hook for assignment that must succeed.
template <class Dict, class T>
struct DictSetOrThrow {
    static void set_or_throw(Dict&, std::string_view, const T&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictSetOrThrow not specialized for this Dict and type");
        grib2marsUnreachable();
    }
};


// ============================================================
//  dict_to_json
// ============================================================
/// @brief Convert a dictionary to JSON using the active traits.
template <typename Dict>
std::string dict_to_json(const Dict& d) {
    return DictToJsonTraits<Dict>::to_json(d);
}

// ============================================================
//  clone / make_from_sample / needs_checks
// ============================================================

/// @brief Report whether a dictionary supports consistency checks.
template <typename Dict>
inline constexpr bool dict_supports_checks_v = DictTraits<Dict>::support_checks;

/// @brief Create a dictionary from a sample name.
template <typename Dict>
std::unique_ptr<Dict> make_from_sample_or_throw(std::string_view name) {
    return DictTraits<Dict>::make_from_sample_or_throw(name);
}

/// @brief Clone a supported dictionary.
template <typename Dict>
std::unique_ptr<Dict> clone_or_throw(const Dict& d) {
    return DictTraits<Dict>::clone_or_throw(d);
}

/// @brief Render a dictionary or ignore unsupported types.
template <typename Dict>
void dump_or_ignore(const Dict& d, const std::string& f) {
    DictToJsonTraits<Dict>::dump_or_ignore(d, f);
}

// ============================================================
//  has / isMissing / setMissing
// ============================================================

// has<Dict>(dict,key)
/// @brief Check whether a key exists in a dictionary.
template <class Dict>
inline bool has(const Dict& dict, std::string_view key) {
    return DictHas<Dict>::has(dict, key);
}

// has<T>(dict,key)
/// @brief Check whether a typed value exists in a dictionary.
template <class T, class Dict>
inline bool has(const Dict& dict, std::string_view key) {
    return DictGetOpt<Dict, T>::get_opt(dict, key).has_value();
}

// isMissing<Dict>(dict,key)
/// @brief Check whether a key is semantically missing.
template <class Dict>
inline bool isMissing(const Dict& dict, std::string_view key) {
    return DictMissing<Dict>::isMissing(dict, key);
}

// setMissing<Dict>(dict,key)
/// @brief Mark a key as missing.
template <class Dict>
inline void setMissing_or_throw(Dict& dict, std::string_view key) {
    DictMissing<Dict>::setMissing(dict, key);
    return;
}

// check<T>(dict,key,cond) -> bool
/// @brief Evaluate a predicate on a typed optional value.
template <class T, class Dict, class Cond>
inline bool check(const Dict& dict, std::string_view key, Cond&& condition) {
    if (auto v = DictGetOpt<Dict, T>::get_opt(dict, key); v.has_value()) {
        return std::forward<Cond>(condition)(*v);
    }
    return false;
}


// ============================================================
//  GET UTILITIES
// ============================================================

// get_or_throw<T>(dict,key) -> T
/// @brief Retrieve a mandatory typed value from a dictionary.
template <class T, class Dict>
inline T get_or_throw(const Dict& dict, std::string_view key) {
    try {
        return DictGetOrThrow<Dict, T>::get_or_throw(dict, key);
    }
    catch (...) {
        std::throw_with_nested(
            exceptions::Grib2MarsDictException("Forwarding errors while getting key `"s + std::string(key) + "` as `" +
                                                   std::string(type_name<T>()) + "` from dictionary`"s,
                                               Here()));
        grib2marsUnreachable();
    }
    grib2marsUnreachable();
}

// get<T>(dict,key) -> std::optional<T>
/// @brief Retrieve an optional typed value from a dictionary.
template <class T, class Dict>
inline std::optional<T> get_opt(const Dict& dict, std::string_view key) {
    try {
        return DictGetOpt<Dict, T>::get_opt(dict, key);
    }
    catch (...) {
        return std::nullopt;
    }
    grib2marsUnreachable();
}


// ============================================================
//  SET UTILITIES
// ============================================================

// set<T>(dict,key,value)
/// @brief Set a typed value in a dictionary and throw on failure.
template <class T, class Dict>
inline void set_or_throw(Dict& dict, std::string_view key, const T& value) {
    try {
        DictSetOrThrow<Dict, T>::set_or_throw(dict, key, value);
        return;
    }
    catch (...) {
        std::throw_with_nested(exceptions::Grib2MarsDictException("Forwarding errors while setting key `"s +
                                                                      std::string(key) + "` as `" +
                                                                      std::string(type_name<T>()) + "` to dictionary`"s,
                                                                  Here()));
        grib2marsUnreachable();
    }
    grib2marsUnreachable();
}

/// @brief Set a typed value in a dictionary and ignore failures.
template <class T, class Dict>
inline void set_or_ignore(Dict& dict, std::string_view key, const T& value) {
    try {
        DictSetOrIgnore<Dict, T>::set_or_ignore(dict, key, value);
        return;
    }
    catch (...) {
        // ignore exceptions
        grib2marsUnreachable();
    }
    grib2marsUnreachable();
}


}  // namespace metkit::grib2mars::utils::dict_traits

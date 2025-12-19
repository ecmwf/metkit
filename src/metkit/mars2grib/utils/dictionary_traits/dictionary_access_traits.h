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

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"
#include "metkit/mars2grib/utils/type_traits_name.h"


namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

template <typename>
struct dependent_false : std::false_type {};


template <typename Dict>
struct DictToJsonTraits {

    static std::string to_json(const Dict&) { return std::string{"[to_json not supported for this dictionary type]"}; }

    static void dump_or_ignore(const Dict&, const std::string&) {
        LOG_DEBUG_LIB(LibMetkit) << to_json(std::declval<Dict>());
    }
};

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

template <class Dict>
struct DictHas {

    static bool has(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictHas not specialized for this Dict");
        __builtin_unreachable();
    }
};


template <class Dict>
struct DictMissing {

    static bool isMissing(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictMissing not specialized for this Dict");
        __builtin_unreachable();
    }

    static void setMissing(Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictMissing not specialized for this Dict");
        __builtin_unreachable();
    }
};

template <class Dict, class T>
struct DictGetOpt {

    static std::optional<T> get_opt(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictGetOpt not specialized for this Dict and type");
        __builtin_unreachable();
    }
};

template <class Dict, class T>
struct DictGetOrThrow {

    static T get_or_throw(const Dict&, std::string_view) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictGetOrThrow not specialized for this Dict and type");
        __builtin_unreachable();
    }
};

template <class Dict, class T>
struct DictSetOrIgnore {
    static void set_or_ignore(Dict&, std::string_view, const T&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictSetOrIgnore not specialized for this Dict and type");
        __builtin_unreachable();
    }
};


template <class Dict, class T>
struct DictSetOrThrow {
    static void set_or_throw(Dict&, std::string_view, const T&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictSetOrThrow not specialized for this Dict and type");
        __builtin_unreachable();
    }
};


// ============================================================
//  dict_to_json
// ============================================================
template <typename Dict>
std::string dict_to_json(const Dict& d) {
    return DictToJsonTraits<Dict>::to_json(d);
}

// ============================================================
//  clone / make_from_sample / needs_checks
// ============================================================

template <typename Dict>
inline constexpr bool dict_supports_checks_v = DictTraits<Dict>::support_checks;

template <typename Dict>
std::unique_ptr<Dict> make_from_sample_or_throw(std::string_view name) {
    return DictTraits<Dict>::make_from_sample_or_throw(name);
}

template <typename Dict>
std::unique_ptr<Dict> clone_or_throw(const Dict& d) {
    return DictTraits<Dict>::clone_or_throw(d);
}

template <typename Dict>
void dump_or_ignore(const Dict& d, const std::string& f) {
    DictToJsonTraits<Dict>::dump_or_ignore(d, f);
}

// ============================================================
//  has / isMissing / setMissing
// ============================================================

// has<Dict>(dict,key)
template <class Dict>
inline bool has(const Dict& dict, std::string_view key) {
    return DictHas<Dict>::has(dict, key);
}

// has<T>(dict,key)
template <class T, class Dict>
inline bool has(const Dict& dict, std::string_view key) {
    return DictGetOpt<Dict, T>::get_opt(dict, key).has_value();
}

// isMissing<Dict>(dict,key)
template <class Dict>
inline bool isMissing(const Dict& dict, std::string_view key) {
    return DictMissing<Dict>::isMissing(dict, key);
}

// setMissing<Dict>(dict,key)
template <class Dict>
inline void setMissing_or_throw(Dict& dict, std::string_view key) {
    DictMissing<Dict>::setMissing(dict, key);
    return;
}

// check<T>(dict,key,cond) -> bool
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
template <class T, class Dict>
inline T get_or_throw(const Dict& dict, std::string_view key) {
    try {
        return DictGetOrThrow<Dict, T>::get_or_throw(dict, key);
    }
    catch (...) {
        std::throw_with_nested(
            exceptions::Mars2GribDictException("Forwarding errors while getting key `"s + std::string(key) + "` as `" +
                                                   std::string(type_name<T>()) + "` from dictionary`"s,
                                               Here()));
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

// get<T>(dict,key) -> std::optional<T>
template <class T, class Dict>
inline std::optional<T> get_opt(const Dict& dict, std::string_view key) {
    try {
        return DictGetOpt<Dict, T>::get_opt(dict, key);
    }
    catch (...) {
        return std::nullopt;
    }
    __builtin_unreachable();
}


// ============================================================
//  SET UTILITIES
// ============================================================

// set<T>(dict,key,value)
template <class T, class Dict>
inline void set_or_throw(Dict& dict, std::string_view key, const T& value) {
    try {
        DictSetOrThrow<Dict, T>::set_or_throw(dict, key, value);
        return;
    }
    catch (...) {
        std::throw_with_nested(exceptions::Mars2GribDictException("Forwarding errors while setting key `"s +
                                                                      std::string(key) + "` as `" +
                                                                      std::string(type_name<T>()) + "` to dictionary`"s,
                                                                  Here()));
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

template <class T, class Dict>
inline void set_or_ignore(Dict& dict, std::string_view key, const T& value) {
    try {
        DictSetOrIgnore<Dict, T>::set_or_ignore(dict, key, value);
        return;
    }
    catch (...) {
        // ignore exceptions
        __builtin_unreachable();
    }
    __builtin_unreachable();
}


}  // namespace metkit::mars2grib::utils::dict_traits

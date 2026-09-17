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
#include "metkit/mars2grib/utils/generalUtils.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"
#include "metkit/mars2grib/utils/type_traits_name.h"


namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

template <typename>
struct dependent_false : std::false_type {};


template <typename Dict>
struct DictToJsonTraits {

    template <class Cntx_t>
    static std::string to_json(const Dict&, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        std::string result{"[to_json not supported for this dictionary type]"};
        profiling::profileExitFunction(cntx, Here());
        return result;
    }

    template <class Cntx_t>
    static void dump_or_ignore(const Dict& dict, const std::string&, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        LOG_DEBUG_LIB(LibMetkit) << to_json(dict, profiling::callSite(cntx, Here()));
        profiling::profileExitFunction(cntx, Here());
    }
};

template <typename Dict>
struct DictTraits {
    static constexpr bool support_checks = false;

    template <class Cntx_t>
    static std::unique_ptr<Dict> make_from_sample_or_throw(std::string_view, Cntx_t&) {
        static_assert(dependent_false<Dict>::value, "DictTraits::make_from_sample_or_throw not specialized");
    }

    template <class Cntx_t>
    static std::unique_ptr<Dict> clone_or_throw(const Dict&, Cntx_t&) {
        static_assert(dependent_false<Dict>::value, "DictTraits::clone_or_throw not specialized");
    }
};

template <class Dict>
struct DictHas {

    template <class Cntx_t>
    static bool has(const Dict&, std::string_view, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictHas not specialized for this Dict");
        mars2gribUnreachable();
    }
};


template <class Dict>
struct DictMissing {

    template <class Cntx_t>
    static bool isMissing(const Dict&, std::string_view, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictMissing not specialized for this Dict");
        mars2gribUnreachable();
    }

    template <class Cntx_t>
    static void setMissing(Dict&, std::string_view, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictMissing not specialized for this Dict");
        mars2gribUnreachable();
    }
};

template <class Dict, class T>
struct DictGetOpt {

    template <class Cntx_t>
    static std::optional<T> get_opt(const Dict&, std::string_view, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictGetOpt not specialized for this Dict and type");
        mars2gribUnreachable();
    }
};

template <class Dict, class T>
struct DictGetOrThrow {

    template <class Cntx_t>
    static T get_or_throw(const Dict&, std::string_view, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictGetOrThrow not specialized for this Dict and type");
        mars2gribUnreachable();
    }
};

template <class Dict, class T>
struct DictSetOrIgnore {
    template <class Cntx_t>
    static void set_or_ignore(Dict&, std::string_view, const T&, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictSetOrIgnore not specialized for this Dict and type");
        mars2gribUnreachable();
    }
};


template <class Dict, class T>
struct DictSetOrThrow {
    template <class Cntx_t>
    static void set_or_throw(Dict&, std::string_view, const T&, Cntx_t&) noexcept(false) {
        static_assert(dependent_false<Dict>::value, "DictSetOrThrow not specialized for this Dict and type");
        mars2gribUnreachable();
    }
};


// ============================================================
//  dict_to_json
// ============================================================
template <typename Dict, class Cntx_t>
std::string dict_to_json(const Dict& d, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    std::string result = DictToJsonTraits<Dict>::to_json(d, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
    return result;
}

// ============================================================
//  clone / make_from_sample / needs_checks
// ============================================================

template <typename Dict>
inline constexpr bool dict_supports_checks_v = DictTraits<Dict>::support_checks;

template <typename Dict, class Cntx_t>
std::unique_ptr<Dict> make_from_sample_or_throw(std::string_view name, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    std::unique_ptr<Dict> result =
        DictTraits<Dict>::make_from_sample_or_throw(name, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
    return result;
}

template <typename Dict, class Cntx_t>
std::unique_ptr<Dict> clone_or_throw(const Dict& d, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    std::unique_ptr<Dict> result = DictTraits<Dict>::clone_or_throw(d, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
    return result;
}

template <typename Dict, class Cntx_t>
void dump_or_ignore(const Dict& d, const std::string& f, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    DictToJsonTraits<Dict>::dump_or_ignore(d, f, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
}

// ============================================================
//  has / isMissing / setMissing
// ============================================================

// has<Dict>(dict,key)
template <class Dict, class Cntx_t>
inline bool has(const Dict& dict, std::string_view key, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    const bool result = DictHas<Dict>::has(dict, key, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
    return result;
}

// has<T>(dict,key)
template <class T, class Dict, class Cntx_t>
inline bool has(const Dict& dict, std::string_view key, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    const bool result = DictGetOpt<Dict, T>::get_opt(dict, key, profiling::callSite(cntx, Here())).has_value();
    profiling::profileExitFunction(cntx, Here());
    return result;
}

// isMissing<Dict>(dict,key)
template <class Dict, class Cntx_t>
inline bool isMissing(const Dict& dict, std::string_view key, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    const bool result = DictMissing<Dict>::isMissing(dict, key, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
    return result;
}

// setMissing<Dict>(dict,key)
template <class Dict, class Cntx_t>
inline void setMissing_or_throw(Dict& dict, std::string_view key, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    DictMissing<Dict>::setMissing(dict, key, profiling::callSite(cntx, Here()));
    profiling::profileExitFunction(cntx, Here());
}

// check<T>(dict,key,cond) -> bool
template <class T, class Dict, class Cond, class Cntx_t>
inline bool check(const Dict& dict, std::string_view key, Cond&& condition, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    bool result = false;
    if (auto v = DictGetOpt<Dict, T>::get_opt(dict, key, profiling::callSite(cntx, Here())); v.has_value()) {
        result = std::forward<Cond>(condition)(*v);
    }
    profiling::profileExitFunction(cntx, Here());
    return result;
}


// ============================================================
//  GET UTILITIES
// ============================================================

// get_or_throw<T>(dict,key) -> T
template <class T, class Dict, class Cntx_t>
inline T get_or_throw(const Dict& dict, std::string_view key, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    try {
        T result = DictGetOrThrow<Dict, T>::get_or_throw(dict, key, profiling::callSite(cntx, Here()));
        profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            exceptions::Mars2GribDictException("Forwarding errors while getting key `"s + std::string(key) + "` as `" +
                                                   std::string(type_name<T>()) + "` from dictionary`"s,
                                               Here()));
        mars2gribUnreachable();
    }
    mars2gribUnreachable();
}

// get<T>(dict,key) -> std::optional<T>
template <class T, class Dict, class Cntx_t>
inline std::optional<T> get_opt(const Dict& dict, std::string_view key, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    try {
        std::optional<T> result = DictGetOpt<Dict, T>::get_opt(dict, key, profiling::callSite(cntx, Here()));
        profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::optional<T> result = std::nullopt;
        profiling::profileExitFunction(cntx, Here());
        return result;
    }
    mars2gribUnreachable();
}


// ============================================================
//  SET UTILITIES
// ============================================================

// set<T>(dict,key,value)
template <class T, class Dict, class Cntx_t>
inline void set_or_throw(Dict& dict, std::string_view key, const T& value, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    try {
        DictSetOrThrow<Dict, T>::set_or_throw(dict, key, value, profiling::callSite(cntx, Here()));
        profiling::profileExitFunction(cntx, Here());
        return;
    }
    catch (...) {
        std::throw_with_nested(exceptions::Mars2GribDictException("Forwarding errors while setting key `"s +
                                                                      std::string(key) + "` as `" +
                                                                      std::string(type_name<T>()) + "` to dictionary`"s,
                                                                  Here()));
        mars2gribUnreachable();
    }
    mars2gribUnreachable();
}

template <class T, class Dict, class Cntx_t>
inline void set_or_ignore(Dict& dict, std::string_view key, const T& value, Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());
    try {
        DictSetOrIgnore<Dict, T>::set_or_ignore(dict, key, value, profiling::callSite(cntx, Here()));
        profiling::profileExitFunction(cntx, Here());
        return;
    }
    catch (...) {
        // ignore exceptions
        mars2gribUnreachable();
    }
    mars2gribUnreachable();
}


}  // namespace metkit::mars2grib::utils::dict_traits

/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/misc-defaults/get_misc_default.h"
#include "metkit/mars2grib/testing-utils/MiscRequirementsDictionary.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::utils {

template <>
constexpr std::string_view type_name<testing_utils::MiscRequirementsDictionary>() {
    return "metkit::mars2grib::testing_utils::MiscRequirementsDictionary";
}

}  // namespace metkit::mars2grib::utils

namespace metkit::mars2grib::utils::dict_traits {

template <>
struct DictHas<testing_utils::MiscRequirementsDictionary> {
    template <class Cntx_t>
    static bool has(const testing_utils::MiscRequirementsDictionary& dict, std::string_view key,
                    Cntx_t& cntx) noexcept(false) {
        profiling::profileEnterFunction(cntx, Here());
        dict.record_optional(key);
        const bool result = false;
        profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

#define M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(CTYPE)                                              \
    template <>                                                                                     \
    struct DictGetOpt<testing_utils::MiscRequirementsDictionary, CTYPE> {                           \
        template <class Cntx_t>                                                                     \
        static std::optional<CTYPE> get_opt(const testing_utils::MiscRequirementsDictionary& dict,  \
                                            std::string_view key, Cntx_t& cntx) noexcept(false) {   \
            profiling::profileEnterFunction(cntx, Here());                                         \
            dict.record_optional<CTYPE>(key);                                                       \
            std::optional<CTYPE> result = std::nullopt;                                             \
            profiling::profileExitFunction(cntx, Here());                                          \
            return result;                                                                          \
        }                                                                                           \
    };                                                                                              \
                                                                                                    \
    template <>                                                                                     \
    struct DictGetOrThrow<testing_utils::MiscRequirementsDictionary, CTYPE> {                       \
        template <class Cntx_t>                                                                     \
        static CTYPE get_or_throw(const testing_utils::MiscRequirementsDictionary& dict,            \
                                  std::string_view key, Cntx_t& cntx) noexcept(false) {             \
            profiling::profileEnterFunction(cntx, Here());                                         \
            const auto value = misc_defaults::get_misc_default<CTYPE>(                              \
                dict.mars(), key, profiling::callSite(cntx, Here()));                               \
            dict.record_mandatory<CTYPE>(key, value);                                               \
            CTYPE result = value.value_or(CTYPE{});                                                 \
            profiling::profileExitFunction(cntx, Here());                                          \
            return result;                                                                          \
        }                                                                                           \
    };

M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(bool)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(long)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(double)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(std::string)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(std::vector<double>)

#undef M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS

template <>
struct DictToJsonTraits<testing_utils::MiscRequirementsDictionary> {
    template <class Cntx_t>
    static std::string to_json(const testing_utils::MiscRequirementsDictionary& dict, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        std::ostringstream out;
        out << "{\"requirements\":[";
        bool first = true;
        for (const auto& requirement : dict.requirements()) {
            if (!first) {
                out << ',';
            }
            first = false;
            out << "{\"key\":\"" << requirement.key
                << "\",\"mandatory\":" << (requirement.mandatory ? "true" : "false");
            if (requirement.type) {
                out << ",\"type\":\"" << *requirement.type << '"';
            }
            if (requirement.defaultValue) {
                out << ",\"default\":\"" << *requirement.defaultValue << '"';
            }
            out << '}';
        }
        out << "]}";
        std::string result = out.str();
        profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

}  // namespace metkit::mars2grib::utils::dict_traits

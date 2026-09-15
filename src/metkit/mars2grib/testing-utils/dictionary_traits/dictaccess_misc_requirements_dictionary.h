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

namespace metkit::mars2grib::utils {

template <>
constexpr std::string_view type_name<testing_utils::MiscRequirementsDictionary>() {
    return "metkit::mars2grib::testing_utils::MiscRequirementsDictionary";
}

}  // namespace metkit::mars2grib::utils

namespace metkit::mars2grib::utils::dict_traits {

template <>
struct DictHas<testing_utils::MiscRequirementsDictionary> {
    static bool has(const testing_utils::MiscRequirementsDictionary& dict, std::string_view key) noexcept(false) {
        dict.record_optional(key);
        return false;
    }
};

#define M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(CTYPE)                                                        \
    template <>                                                                                                \
    struct DictGetOpt<testing_utils::MiscRequirementsDictionary, CTYPE> {                                     \
        static std::optional<CTYPE> get_opt(const testing_utils::MiscRequirementsDictionary& dict,           \
                                            std::string_view key) noexcept(false) {                            \
            dict.record_optional<CTYPE>(key);                                                                 \
            return std::nullopt;                                                                               \
        }                                                                                                      \
    };                                                                                                         \
                                                                                                               \
    template <>                                                                                                \
    struct DictGetOrThrow<testing_utils::MiscRequirementsDictionary, CTYPE> {                                 \
        static CTYPE get_or_throw(const testing_utils::MiscRequirementsDictionary& dict,                      \
                                  std::string_view key) noexcept(false) {                                      \
            const auto value = misc_defaults::get_misc_default<CTYPE>(dict.mars(), key);                      \
            dict.record_mandatory<CTYPE>(key, value);                                                         \
            return value.value_or(CTYPE{});                                                                   \
        }                                                                                                      \
    };

M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(bool)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(long)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(double)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(std::string)
M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS(std::vector<double>)

#undef M2G_DEFINE_MISC_REQUIREMENTS_GET_TRAITS

template <>
struct DictToJsonTraits<testing_utils::MiscRequirementsDictionary> {
    static std::string to_json(const testing_utils::MiscRequirementsDictionary& dict) {
        std::ostringstream out;
        out << "{\"requirements\":[";
        bool first = true;
        for (const auto& requirement : dict.requirements()) {
            if (!first) {
                out << ',';
            }
            first = false;
            out << "{\"key\":\"" << requirement.key << "\",\"mandatory\":"
                << (requirement.mandatory ? "true" : "false");
            if (requirement.type) {
                out << ",\"type\":\"" << *requirement.type << '"';
            }
            if (requirement.defaultValue) {
                out << ",\"default\":\"" << *requirement.defaultValue << '"';
            }
            out << '}';
        }
        out << "]}";
        return out.str();
    }
};

}  // namespace metkit::mars2grib::utils::dict_traits

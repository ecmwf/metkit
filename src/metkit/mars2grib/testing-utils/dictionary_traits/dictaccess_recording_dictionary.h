#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/codes/api/CodesTypes.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/type_traits_name.h"

namespace metkit::mars2grib::utils {

template <>
constexpr std::string_view type_name<metkit::mars2grib::testing_utils::RecordingDictionary>() {
    return "metkit::mars2grib::testing_utils::RecordingDictionary";
}

}  // namespace metkit::mars2grib::utils

namespace metkit::mars2grib::utils::dict_traits {

template <>
struct DictToJsonTraits<metkit::mars2grib::testing_utils::RecordingDictionary> {

    static std::string to_json(const metkit::mars2grib::testing_utils::RecordingDictionary& dict) {
        return dict.to_json();
    }

    static void dump_or_ignore(const metkit::mars2grib::testing_utils::RecordingDictionary& dict,
                               const std::string& fname) {
        try {
            std::ofstream out(fname, std::ios::out | std::ios::trunc);
            if (!out) {
                return;
            }
            out << dict.to_json();
            out.flush();
        }
        catch (...) {
            return;
        }
    }
};

template <>
struct DictTraits<metkit::mars2grib::testing_utils::RecordingDictionary> {
    static constexpr bool support_checks = false;

    static std::unique_ptr<metkit::mars2grib::testing_utils::RecordingDictionary> make_from_sample_or_throw(
        std::string_view name) {
        auto dict = std::make_unique<metkit::mars2grib::testing_utils::RecordingDictionary>();
        dict->record_make_from_sample(name);
        return dict;
    }

    static std::unique_ptr<metkit::mars2grib::testing_utils::RecordingDictionary> clone_or_throw(
        const metkit::mars2grib::testing_utils::RecordingDictionary& dict) {
        auto clone = std::make_unique<metkit::mars2grib::testing_utils::RecordingDictionary>(dict);
        clone->record_clone(dict.operation_count());
        return clone;
    }
};

template <>
struct DictMissing<metkit::mars2grib::testing_utils::RecordingDictionary> {
    static bool isMissing(const metkit::mars2grib::testing_utils::RecordingDictionary&,
                          std::string_view) noexcept(false) {
        throw std::logic_error("RecordingDictionary does not support read-side missing checks");
    }

    static void setMissing(metkit::mars2grib::testing_utils::RecordingDictionary& dict,
                           std::string_view key) noexcept(false) {
        dict.record_set_missing(key);
    }
};

#define M2G_DEFINE_RECORDING_DICT_SET_TRAITS(CTYPE)                                                                  \
    template <>                                                                                                      \
    struct DictSetOrThrow<metkit::mars2grib::testing_utils::RecordingDictionary, CTYPE> {                            \
        static void set_or_throw(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,  \
                                 const CTYPE& value) noexcept(false) {                                               \
            dict.record_set(key, value);                                                                             \
        }                                                                                                            \
    };                                                                                                               \
                                                                                                                     \
    template <>                                                                                                      \
    struct DictSetOrIgnore<metkit::mars2grib::testing_utils::RecordingDictionary, CTYPE> {                           \
        static void set_or_ignore(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key, \
                                  const CTYPE& value) noexcept(false) {                                              \
            dict.record_set(key, value);                                                                             \
        }                                                                                                            \
    };

M2G_DEFINE_RECORDING_DICT_SET_TRAITS(bool)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(int)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(long)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(double)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(std::string)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(std::vector<long>)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(std::vector<double>)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(std::vector<std::string>)
M2G_DEFINE_RECORDING_DICT_SET_TRAITS(std::vector<uint8_t>)

template <>
struct DictSetOrThrow<metkit::mars2grib::testing_utils::RecordingDictionary, metkit::codes::Span<const double>> {
    static void set_or_throw(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,
                             const metkit::codes::Span<const double>& value) noexcept(false) {
        std::vector<double> owned;
        owned.reserve(value.size());
        const double* raw = value.data();
        for (std::size_t i = 0; i < value.size(); ++i) {
            owned.push_back(raw[i]);
        }
        dict.record_set(key, std::move(owned));
    }
};

template <>
struct DictSetOrIgnore<metkit::mars2grib::testing_utils::RecordingDictionary, metkit::codes::Span<const double>> {
    static void set_or_ignore(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,
                              const metkit::codes::Span<const double>& value) noexcept(false) {
        std::vector<double> owned;
        owned.reserve(value.size());
        const double* raw = value.data();
        for (std::size_t i = 0; i < value.size(); ++i) {
            owned.push_back(raw[i]);
        }
        dict.record_set(key, std::move(owned));
    }
};

#undef M2G_DEFINE_RECORDING_DICT_SET_TRAITS

}  // namespace metkit::mars2grib::utils::dict_traits

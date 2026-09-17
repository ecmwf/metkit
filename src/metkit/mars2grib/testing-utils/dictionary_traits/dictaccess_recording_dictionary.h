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
#include "metkit/mars2grib/utils/profiling/Profiling.h"
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

    template <class Cntx_t>
    static std::string to_json(const metkit::mars2grib::testing_utils::RecordingDictionary& dict, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        std::string result = dict.to_json();
        profiling::profileExitFunction(cntx, Here());
        return result;
    }

    template <class Cntx_t>
    static void dump_or_ignore(const metkit::mars2grib::testing_utils::RecordingDictionary& dict,
                               const std::string& fname, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        try {
            std::ofstream out(fname, std::ios::out | std::ios::trunc);
            if (!out) {
                profiling::profileExitFunction(cntx, Here());
                return;
            }
            out << dict.to_json();
            out.flush();
        }
        catch (...) {
            profiling::profileExitFunction(cntx, Here());
            return;
        }
        profiling::profileExitFunction(cntx, Here());
    }
};

template <>
struct DictTraits<metkit::mars2grib::testing_utils::RecordingDictionary> {
    static constexpr bool support_checks = false;

    template <class Cntx_t>
    static std::unique_ptr<metkit::mars2grib::testing_utils::RecordingDictionary> make_from_sample_or_throw(
        std::string_view name, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        std::unique_ptr<metkit::mars2grib::testing_utils::RecordingDictionary> result =
            std::make_unique<metkit::mars2grib::testing_utils::RecordingDictionary>();
        result->record_make_from_sample(name);
        profiling::profileExitFunction(cntx, Here());
        return result;
    }

    template <class Cntx_t>
    static std::unique_ptr<metkit::mars2grib::testing_utils::RecordingDictionary> clone_or_throw(
        const metkit::mars2grib::testing_utils::RecordingDictionary& dict, Cntx_t& cntx) {
        profiling::profileEnterFunction(cntx, Here());
        std::unique_ptr<metkit::mars2grib::testing_utils::RecordingDictionary> result =
            std::make_unique<metkit::mars2grib::testing_utils::RecordingDictionary>(dict);
        result->record_clone(dict.operation_count());
        profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

template <>
struct DictMissing<metkit::mars2grib::testing_utils::RecordingDictionary> {
    template <class Cntx_t>
    static bool isMissing(const metkit::mars2grib::testing_utils::RecordingDictionary&,
                          std::string_view, Cntx_t& cntx) noexcept(false) {
        profiling::profileEnterFunction(cntx, Here());
        throw std::logic_error("RecordingDictionary does not support read-side missing checks");
    }

    template <class Cntx_t>
    static void setMissing(metkit::mars2grib::testing_utils::RecordingDictionary& dict,
                           std::string_view key, Cntx_t& cntx) noexcept(false) {
        profiling::profileEnterFunction(cntx, Here());
        dict.record_set_missing(key);
        profiling::profileExitFunction(cntx, Here());
    }
};

#define M2G_DEFINE_RECORDING_DICT_SET_TRAITS(CTYPE)                                                                   \
    template <>                                                                                                       \
    struct DictSetOrThrow<metkit::mars2grib::testing_utils::RecordingDictionary, CTYPE> {                             \
        template <class Cntx_t>                                                                                       \
        static void set_or_throw(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,   \
                                 const CTYPE& value, Cntx_t& cntx) noexcept(false) {                                  \
            profiling::profileEnterFunction(cntx, Here());                                                           \
            dict.record_set(key, value);                                                                              \
            profiling::profileExitFunction(cntx, Here());                                                            \
        }                                                                                                             \
    };                                                                                                                \
                                                                                                                      \
    template <>                                                                                                       \
    struct DictSetOrIgnore<metkit::mars2grib::testing_utils::RecordingDictionary, CTYPE> {                            \
        template <class Cntx_t>                                                                                       \
        static void set_or_ignore(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,  \
                                  const CTYPE& value, Cntx_t& cntx) noexcept(false) {                                 \
            profiling::profileEnterFunction(cntx, Here());                                                           \
            dict.record_set(key, value);                                                                              \
            profiling::profileExitFunction(cntx, Here());                                                            \
        }                                                                                                             \
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
    template <class Cntx_t>
    static void set_or_throw(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,
                             const metkit::codes::Span<const double>& value, Cntx_t& cntx) noexcept(false) {
        profiling::profileEnterFunction(cntx, Here());
        auto span = value;
        std::vector<double> owned;
        owned.reserve(span.size());
        const double* raw = span.data();
        for (std::size_t i = 0; i < span.size(); ++i) {
            owned.push_back(raw[i]);
        }
        dict.record_set(key, std::move(owned));
        profiling::profileExitFunction(cntx, Here());
    }
};

template <>
struct DictSetOrIgnore<metkit::mars2grib::testing_utils::RecordingDictionary, metkit::codes::Span<const double>> {
    template <class Cntx_t>
    static void set_or_ignore(metkit::mars2grib::testing_utils::RecordingDictionary& dict, std::string_view key,
                              const metkit::codes::Span<const double>& value, Cntx_t& cntx) noexcept(false) {
        profiling::profileEnterFunction(cntx, Here());
        auto span = value;
        std::vector<double> owned;
        owned.reserve(span.size());
        const double* raw = span.data();
        for (std::size_t i = 0; i < span.size(); ++i) {
            owned.push_back(raw[i]);
        }
        dict.record_set(key, std::move(owned));
        profiling::profileExitFunction(cntx, Here());
    }
};

#undef M2G_DEFINE_RECORDING_DICT_SET_TRAITS

}  // namespace metkit::mars2grib::utils::dict_traits

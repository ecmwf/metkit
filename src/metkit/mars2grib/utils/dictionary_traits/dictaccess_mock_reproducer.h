#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/codes/api/CodesTypes.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/type_traits_name.h"
#include "metkit/mars2grib/debug/MockupDictionary.h"

namespace metkit::mars2grib::utils {

using MockDictionary = metkit::mars2grib::debug::reproducer::MockDictionary;


template <>
constexpr std::string_view type_name<MockDictionary>() {
    return "metkit::mars2grib::debug::MockDictionary";
}

}  // namespace metkit::mars2grib::utils

namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

#define M2G_DEFINE_MOCK_DICT_GET_OR_THROW(CTYPE)                                               \
    template <>                                                                                \
    struct DictGetOrThrow<MockDictionary, CTYPE> {                                             \
        static CTYPE get_or_throw(const MockDictionary&, std::string_view key) noexcept(false) { \
            throw exceptions::Mars2GribDictException(                                          \
                "Cannot read key `"s + std::string(key) + "` from write-only MockDictionary",  \
                Here());                                                                       \
        }                                                                                      \
    };

#define M2G_DEFINE_MOCK_DICT_GET_OPT(CTYPE)                                                    \
    template <>                                                                                \
    struct DictGetOpt<MockDictionary, CTYPE> {                                                 \
        static std::optional<CTYPE> get_opt(const MockDictionary&, std::string_view) noexcept(false) { \
            return std::nullopt;                                                               \
        }                                                                                      \
    };

#define M2G_DEFINE_MOCK_DICT_SET_OR_THROW(CTYPE, SETFUNC)                                      \
    template <>                                                                                \
    struct DictSetOrThrow<MockDictionary, CTYPE> {                                             \
        static void set_or_throw(MockDictionary& d, std::string_view key,                      \
                                 const CTYPE& v) noexcept(false) {                             \
            const std::string k{key};                                                          \
            try {                                                                              \
                d.SETFUNC(k, v);                                                               \
            } catch (const std::exception& e) {                                                \
                std::throw_with_nested(exceptions::Mars2GribDictException(                     \
                    "Mock recorder failed on key `"s + k + "`: "s + e.what(), Here()));        \
            }                                                                                  \
        }                                                                                      \
    };

#define M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(CTYPE, SETFUNC)                                     \
    template <>                                                                                \
    struct DictSetOrIgnore<MockDictionary, CTYPE> {                                            \
        static void set_or_ignore(MockDictionary& d, std::string_view key,                     \
                                  const CTYPE& v) noexcept(false) {                            \
            try { d.SETFUNC(std::string{key}, v); } catch (...) {}                             \
        }                                                                                      \
    };

template <>
struct DictToJsonTraits<MockDictionary> {
    static std::string to_json(const MockDictionary&) {
        return std::string{"[to_json not supported for MockDictionary]"};
    }
    static void dump_or_ignore(const MockDictionary& d, const std::string& fname) {
        d.dump(fname);
    }
};

// -----------------------------------------------------------------------------
// DictTraits (Linear Shared Architecture)
// -----------------------------------------------------------------------------
template <>
struct DictTraits<MockDictionary> {
    static constexpr bool support_checks = false;

    // Starts the timeline
    static std::unique_ptr<MockDictionary> make_from_sample_or_throw(std::string_view name) {
        auto rec = std::make_shared<Reproducer::Recorder>();
        rec->record_from_sample(std::string(name));
        return std::make_unique<MockDictionary>(rec);
    }

    // Injects a "flush" operation into the shared timeline
    static std::unique_ptr<MockDictionary> clone_or_throw(const MockDictionary& other) {
        other.recorder->record_clone();
        return std::make_unique<MockDictionary>(other.recorder);
    }
};

template <>
struct DictHas<MockDictionary> {
    static bool has(const MockDictionary&, std::string_view key) noexcept(false) {
        throw exceptions::Mars2GribDictException("Cannot check key presence in MockDictionary", Here());
    }
};

template <>
struct DictMissing<MockDictionary> {
    static bool isMissing(const MockDictionary&, std::string_view key) noexcept(false) {
        throw exceptions::Mars2GribDictException("Cannot check missing state in MockDictionary", Here());
    }
    static void setMissing(MockDictionary& d, std::string_view key) noexcept(false) {
        try { d.recorder->record_missing(std::string(key)); }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException("Unable to set missing", Here()));
        }
    }
};

// ============================================================================
// SCALAR TYPES
// ============================================================================
M2G_DEFINE_MOCK_DICT_GET_OR_THROW(bool)
M2G_DEFINE_MOCK_DICT_GET_OPT(bool)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(bool, set)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(bool, set)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(int)
M2G_DEFINE_MOCK_DICT_GET_OPT(int)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(int, set)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(int, set)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(long)
M2G_DEFINE_MOCK_DICT_GET_OPT(long)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(long, set)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(long, set)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(double)
M2G_DEFINE_MOCK_DICT_GET_OPT(double)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(double, set)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(double, set)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(std::string)
M2G_DEFINE_MOCK_DICT_GET_OPT(std::string)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(std::string, set)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(std::string, set)

// ============================================================================
// VECTOR TYPES
// ============================================================================
M2G_DEFINE_MOCK_DICT_GET_OR_THROW(std::vector<long>)
M2G_DEFINE_MOCK_DICT_GET_OPT(std::vector<long>)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(std::vector<long>, set_array)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(std::vector<long>, set_array)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(std::vector<double>)
M2G_DEFINE_MOCK_DICT_GET_OPT(std::vector<double>)

M2G_DEFINE_MOCK_DICT_SET_OR_THROW(metkit::codes::Span<const double>, set_array)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(std::vector<double>, set_array)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(metkit::codes::Span<const double>, set_array)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(std::vector<double>, set_array)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(std::vector<std::string>)
M2G_DEFINE_MOCK_DICT_GET_OPT(std::vector<std::string>)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(std::vector<std::string>, set_array)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(std::vector<std::string>, set_array)

M2G_DEFINE_MOCK_DICT_GET_OR_THROW(std::vector<uint8_t>)
M2G_DEFINE_MOCK_DICT_GET_OPT(std::vector<uint8_t>)
M2G_DEFINE_MOCK_DICT_SET_OR_THROW(std::vector<uint8_t>, set_array)
M2G_DEFINE_MOCK_DICT_SET_OR_IGNORE(std::vector<uint8_t>, set_array)

}  // namespace metkit::mars2grib::utils::dict_traits
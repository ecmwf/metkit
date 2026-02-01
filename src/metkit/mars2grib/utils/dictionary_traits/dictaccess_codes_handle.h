#pragma once

#include <cstdint>
#include <exception>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "eckit/io/Buffer.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/CodesTypes.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"
#include "metkit/mars2grib/utils/type_traits_name.h"


// ============================================================================
// Helper macros to reduce boilerplate (similar to LocalConfiguration version)
// ============================================================================

// ==============================================================
// GET_OR_THROW
// ==============================================================
#define M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(CTYPE, CHECK_NATIVE, GETFUNC)                                 \
    template <>                                                                                                \
    struct DictGetOrThrow<metkit::codes::CodesHandle, CTYPE> {                                                 \
                                                                                                               \
        static CTYPE get_or_throw(const metkit::codes::CodesHandle& h, std::string_view key) noexcept(false) { \
            const std::string k{key};                                                                          \
            try {                                                                                              \
                /* key exists? */                                                                              \
                if (!h.isDefined(k)) {                                                                         \
                    throw exceptions::Mars2GribDictException(                                                  \
                        "Missing key `"s + k + "` in dictionary `"s +                                          \
                            std::string(type_name<metkit::codes::CodesHandle>()) + "`",                        \
                        Here());                                                                               \
                }                                                                                              \
                                                                                                               \
                /* check type */                                                                               \
                auto t = h.type(k);                                                                            \
                if (!(CHECK_NATIVE)) {                                                                         \
                    throw exceptions::Mars2GribDictException(                                                  \
                        "Key `"s + k + "` is not of expected type `"s + std::string(type_name<CTYPE>()) +      \
                            "` in dictionary `"s + std::string(type_name<metkit::codes::CodesHandle>()) + "`", \
                        Here());                                                                               \
                }                                                                                              \
                                                                                                               \
                return h.GETFUNC(k);                                                                           \
            }                                                                                                  \
            catch (const exceptions::Mars2GribGenericException&) {                                             \
                throw;                                                                                         \
            }                                                                                                  \
            catch (...) {                                                                                      \
                std::throw_with_nested(exceptions::Mars2GribDictException(                                     \
                    "Internal error while reading key `"s + k + "` as `" + std::string(type_name<CTYPE>()) +   \
                        "` from dictionary `"s + std::string(type_name<metkit::codes::CodesHandle>()) + "`",   \
                    Here()));                                                                                  \
            }                                                                                                  \
            __builtin_unreachable();                                                                           \
        }                                                                                                      \
    };


// ==============================================================
// GET_OPT
// ==============================================================
#define M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(CTYPE, CHECK_NATIVE, GETFUNC)           \
    template <>                                                                     \
    struct DictGetOpt<metkit::codes::CodesHandle, CTYPE> {                          \
                                                                                    \
        static std::optional<CTYPE> get_opt(const metkit::codes::CodesHandle& h,    \
                                            std::string_view key) noexcept(false) { \
            const std::string k{key};                                               \
            try {                                                                   \
                if (!h.isDefined(k))                                                \
                    return std::nullopt;                                            \
                                                                                    \
                auto t = h.type(k);                                                 \
                if (!(CHECK_NATIVE))                                                \
                    return std::nullopt;                                            \
                                                                                    \
                return h.GETFUNC(k);                                                \
            }                                                                       \
            catch (...) {                                                           \
                return std::nullopt;                                                \
            }                                                                       \
            __builtin_unreachable();                                                \
        }                                                                           \
    };


// ==============================================================
// SET_OR_THROW
// ==============================================================
#define M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(CTYPE, SETFUNC)                                           \
    template <>                                                                                            \
    struct DictSetOrThrow<metkit::codes::CodesHandle, CTYPE> {                                             \
                                                                                                           \
        static void set_or_throw(metkit::codes::CodesHandle& h, std::string_view key,                      \
                                 const CTYPE& v) noexcept(false) {                                         \
            const std::string k{key};                                                                      \
            try {                                                                                          \
                h.SETFUNC(k, v);                                                                           \
                return;                                                                                    \
            }                                                                                              \
            catch (const exceptions::Mars2GribGenericException&) {                                         \
                throw;                                                                                     \
            }                                                                                              \
            catch (...) {                                                                                  \
                std::throw_with_nested(exceptions::Mars2GribDictException(                                 \
                    "Unable to set key `"s + k + "` with type `"s + std::string(type_name<CTYPE>()) +      \
                        "` in dictionary `"s + std::string(type_name<metkit::codes::CodesHandle>()) + "`", \
                    Here()));                                                                              \
            }                                                                                              \
            __builtin_unreachable();                                                                       \
        }                                                                                                  \
    };


// ==============================================================
// SET_OR_IGNORE
// ==============================================================
#define M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(CTYPE, SETFUNC)                      \
    template <>                                                                        \
    struct DictSetOrIgnore<metkit::codes::CodesHandle, CTYPE> {                        \
                                                                                       \
        static void set_or_ignore(metkit::codes::CodesHandle& h, std::string_view key, \
                                  const CTYPE& v) noexcept(false) {                    \
            const std::string k{key};                                                  \
            try {                                                                      \
                h.SETFUNC(k, v);                                                       \
                return;                                                                \
            }                                                                          \
            catch (...) {                                                              \
                return;                                                                \
            }                                                                          \
            __builtin_unreachable();                                                   \
        }                                                                              \
    };


namespace metkit::mars2grib::utils {

// -----------------------------------------------------------------------------
// type_name specialisation
// -----------------------------------------------------------------------------
template <>
constexpr std::string_view type_name<metkit::codes::CodesHandle>() {
    return "metkit::codes::CodesHandle";
}

}  // namespace metkit::mars2grib::utils


namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

// -----------------------------------------------------------------------------
// DictToJsonTraits
// -----------------------------------------------------------------------------
template <>
struct DictToJsonTraits<metkit::codes::CodesHandle> {

    static std::string to_json(const metkit::codes::CodesHandle& sample) {
        return std::string{"[to_json not supported for codeHandle this dictionary type]"};
    }

    static void dump_or_ignore(const metkit::codes::CodesHandle& sample, const std::string& fname) {

        try {
            eckit::Buffer buf{sample.messageSize()};
            sample.copyInto(reinterpret_cast<uint8_t*>(buf.data()), buf.size());

            std::ofstream out(fname, std::ios::binary | std::ios::out);
            if (!out) {
                return;  // fail silently
            }

            out.write(static_cast<const char*>(buf.data()), buf.size());
            out.flush();
            // ofstream destructor closes the file
        }
        catch (...) {
            LOG_DEBUG_LIB(LibMetkit) << "dump_or_ignore: unable to dump CodesHandle to file " << fname << std::endl;
            // nothrow guarantee: swallow everything
        }
    }
};

// -----------------------------------------------------------------------------
// DictCreateFromSample / Clone / NeedsChecks
// -----------------------------------------------------------------------------
template <>
struct DictTraits<metkit::codes::CodesHandle> {

    static constexpr bool support_checks = true;

    static std::unique_ptr<metkit::codes::CodesHandle> make_from_sample_or_throw(std::string_view name) {

        auto h = metkit::codes::codesHandleFromSample(std::string(name));
        if (!h) {
            throw eckit::SeriousBug("codesHandleFromSample failed", Here());
        }
        return h;
    }

    static std::unique_ptr<metkit::codes::CodesHandle> clone_or_throw(const metkit::codes::CodesHandle& h) {
        return h.clone();
    }
};

// -----------------------------------------------------------------------------
// DictHas
// -----------------------------------------------------------------------------
template <>
struct DictHas<metkit::codes::CodesHandle> {

    static bool has(const metkit::codes::CodesHandle& h, std::string_view key) noexcept(false) {
        try {
            return h.has(std::string(key));
        }
        catch (const exceptions::Mars2GribGenericException&) {
            throw;
        }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException(
                "Internal error while checking presence of key `"s + std::string(key) + "` in dictionary type `"s +
                    std::string(type_name<metkit::codes::CodesHandle>()) + "`",
                Here()));
        }
        __builtin_unreachable();
    }
};


// -----------------------------------------------------------------------------
// DictMissing
// -----------------------------------------------------------------------------
template <>
struct DictMissing<metkit::codes::CodesHandle> {

    static bool isMissing(const metkit::codes::CodesHandle& h, std::string_view key) noexcept(false) {
        try {
            return h.isMissing(std::string(key));
        }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException(
                "Internal error while checking missing state of key `"s + std::string(key) + "` in dictionary type `"s +
                    std::string(type_name<metkit::codes::CodesHandle>()) + "`",
                Here()));
        }
        __builtin_unreachable();
    }

    static void setMissing(metkit::codes::CodesHandle& h, std::string_view key) noexcept(false) {
        try {
            h.setMissing(std::string(key));
            return;
        }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException(
                "Unable to set key `"s + std::string(key) + "` to missing in dictionary type `"s +
                    std::string(type_name<metkit::codes::CodesHandle>()) + "`",
                Here()));
        }
        __builtin_unreachable();
    }
};


// ============================================================================
// SCALAR TYPES
// ============================================================================

// bool (ECCODES does not have a native boolean; treat as long {0,1})
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(bool, (t == metkit::codes::NativeType::Long && h.size(k) == 1), getLong)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(bool, (t == metkit::codes::NativeType::Long && h.size(k) == 1), getLong)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(bool, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(bool, set)


// int
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(int, (t == metkit::codes::NativeType::Long && h.size(k) == 1), getLong)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(int, (t == metkit::codes::NativeType::Long && h.size(k) == 1), getLong)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(int, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(int, set)


// long
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(long, (t == metkit::codes::NativeType::Long && h.size(k) == 1), getLong)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(long, (t == metkit::codes::NativeType::Long && h.size(k) == 1), getLong)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(long, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(long, set)


// double
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(double, (t == metkit::codes::NativeType::Double && h.size(k) == 1), getDouble)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(double, (t == metkit::codes::NativeType::Double && h.size(k) == 1), getDouble)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(double, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(double, set)


// string
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(std::string, (t == metkit::codes::NativeType::String), getString)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(std::string, (t == metkit::codes::NativeType::String), getString)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(std::string, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(std::string, set)


// ============================================================================
// VECTOR TYPES
// ============================================================================

// std::vector<long>
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(std::vector<long>, (t == metkit::codes::NativeType::Long), getLongArray)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(std::vector<long>, (t == metkit::codes::NativeType::Long), getLongArray)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(std::vector<long>, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(std::vector<long>, set)


// std::vector<double>
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(std::vector<double>, (t == metkit::codes::NativeType::Double), getDoubleArray)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(std::vector<double>, (t == metkit::codes::NativeType::Double), getDoubleArray)


M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(metkit::codes::Span<const double>, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(std::vector<double>, set)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(metkit::codes::Span<const double>, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(std::vector<double>, set)


// std::vector<std::string>
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(std::vector<std::string>, (t == metkit::codes::NativeType::String),
                                         getStringArray)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(std::vector<std::string>, (t == metkit::codes::NativeType::String), getStringArray)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(std::vector<std::string>, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(std::vector<std::string>, set)


// std::vector<uint8_t>
M2G_DEFINE_CODESHANDLE_DICT_GET_OR_THROW(std::vector<uint8_t>, (t == metkit::codes::NativeType::Bytes), getBytes)

M2G_DEFINE_CODESHANDLE_DICT_GET_OPT(std::vector<uint8_t>, (t == metkit::codes::NativeType::Bytes), getBytes)

M2G_DEFINE_CODESHANDLE_DICT_SET_OR_THROW(std::vector<uint8_t>, set)
M2G_DEFINE_CODESHANDLE_DICT_SET_OR_IGNORE(std::vector<uint8_t>, set)


}  // namespace metkit::mars2grib::utils::dict_traits

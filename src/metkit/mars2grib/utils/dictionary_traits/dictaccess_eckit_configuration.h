#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"
#include "metkit/mars2grib/utils/type_traits_name.h"


// ============================================================================
// Helper macros to reduce boilerplate (similar to LocalConfiguration version)
// ============================================================================

// ==============================================================
// GET_OR_THROW
// ==============================================================
#define M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(CTYPE, ISFUNC, GETFUNC)                                               \
    template <>                                                                                                        \
    struct DictGetOrThrow<eckit::LocalConfiguration, CTYPE> {                                                          \
                                                                                                                       \
        static CTYPE get_or_throw(const eckit::LocalConfiguration& cfg, std::string_view key) noexcept(false) {        \
            const std::string k{key};                                                                                  \
                                                                                                                       \
            try {                                                                                                      \
                /* Check key exists */                                                                                 \
                if (!cfg.has(k)) {                                                                                     \
                    throw exceptions::Mars2GribDictException("Missing key `"s + k + "` in dictionary type `"s +        \
                                                                 std::string(type_name<eckit::LocalConfiguration>()) + \
                                                                 "`",                                                  \
                                                             Here());                                                  \
                }                                                                                                      \
                                                                                                                       \
                /* Check type */                                                                                       \
                if (hacks::ISFUNC(cfg, k)) {                                                                           \
                    return cfg.GETFUNC(k);                                                                             \
                }                                                                                                      \
                else {                                                                                                 \
                    throw exceptions::Mars2GribDictException(                                                          \
                        "Key `"s + k + "` is not of expected type `"s + std::string(type_name<CTYPE>()) +              \
                            "` for dictionary type `"s + std::string(type_name<eckit::LocalConfiguration>()) + "`",    \
                        Here());                                                                                       \
                }                                                                                                      \
            }                                                                                                          \
            catch (const exceptions::Mars2GribGenericException&) {                                                     \
                throw;                                                                                                 \
            }                                                                                                          \
            catch (...) {                                                                                              \
                std::throw_with_nested(exceptions::Mars2GribDictException(                                             \
                    "Internal error while reading key `"s + k + "` as `" + std::string(type_name<CTYPE>()) +           \
                        "` from dictionary type `"s + std::string(type_name<eckit::LocalConfiguration>()) + "`",       \
                    Here()));                                                                                          \
            }                                                                                                          \
            __builtin_unreachable();                                                                                   \
        }                                                                                                              \
    };

// ==============================================================
// GET_OPT
// ==============================================================
#define M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(CTYPE, ISFUNC, GETFUNC)                 \
    template <>                                                                     \
    struct DictGetOpt<eckit::LocalConfiguration, CTYPE> {                           \
        static std::optional<CTYPE> get_opt(const eckit::LocalConfiguration& cfg,   \
                                            std::string_view key) noexcept(false) { \
            const std::string k{key};                                               \
                                                                                    \
            try {                                                                   \
                if (!cfg.has(k)) {                                                  \
                    return std::nullopt;                                            \
                }                                                                   \
                                                                                    \
                if (hacks::ISFUNC(cfg, k)) {                                        \
                    return cfg.GETFUNC(k);                                          \
                }                                                                   \
                else {                                                              \
                    return std::nullopt;                                            \
                }                                                                   \
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
#define M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(CTYPE, SETFUNC)                       \
    template <>                                                                         \
    struct DictSetOrIgnore<eckit::LocalConfiguration, CTYPE> {                          \
        static void set_or_ignore(eckit::LocalConfiguration& cfg, std::string_view key, \
                                  const CTYPE& value) noexcept(false) {                 \
            const std::string k{key};                                                   \
                                                                                        \
            try {                                                                       \
                cfg.SETFUNC(k, value);                                                  \
                return;                                                                 \
            }                                                                           \
            catch (...) {                                                               \
            }                                                                           \
            __builtin_unreachable();                                                    \
        }                                                                               \
    };


// ==============================================================
// SET_OR_IGNORE
// ==============================================================
#define M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(CTYPE, SETFUNC)                                               \
    template <>                                                                                                \
    struct DictSetOrThrow<eckit::LocalConfiguration, CTYPE> {                                                  \
                                                                                                               \
        static void set_or_throw(eckit::LocalConfiguration& cfg, std::string_view key,                         \
                                 const CTYPE& value) noexcept(false) {                                         \
            const std::string k{key};                                                                          \
                                                                                                               \
            try {                                                                                              \
                cfg.SETFUNC(k, value);                                                                         \
                return;                                                                                        \
            }                                                                                                  \
            catch (const exceptions::Mars2GribGenericException&) {                                             \
                throw;                                                                                         \
            }                                                                                                  \
            catch (...) {                                                                                      \
                std::throw_with_nested(exceptions::Mars2GribDictException(                                     \
                    "Unable to set key `"s + k + "` with type `"s + std::string(type_name<CTYPE>()) +          \
                        "` in dictionary type `"s + std::string(type_name<eckit::LocalConfiguration>()) + "`", \
                    Here()));                                                                                  \
            }                                                                                                  \
            __builtin_unreachable();                                                                           \
        }                                                                                                      \
    };

namespace metkit::mars2grib::utils {

// -----------------------------------------------------------------------------
// type_name specialisation
// -----------------------------------------------------------------------------
template <>
constexpr std::string_view type_name<eckit::LocalConfiguration>() {
    return "eckit::LocalConfiguration";
}

template <>
constexpr std::string_view type_name<std::vector<eckit::LocalConfiguration>>() {
    return "vector<eckit::LocalConfiguration>";
}

}  // namespace metkit::mars2grib::utils


namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

namespace hacks {

inline bool isIntegral(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isIntegral(std::string{key});
}

inline bool isFloatingPoint(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isFloatingPoint(std::string{key}) || conf.isIntegral(std::string{key});
}

inline bool isBoolean(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isBoolean(std::string{key});
}

inline bool isString(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isString(std::string{key});
}

inline bool isSubConfiguration(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isSubConfiguration(std::string{key});
}

inline bool isIntegralList(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isIntegralList(std::string{key});
}

inline bool isFloatingPointList(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isFloatingPointList(std::string{key}) || conf.isIntegralList(std::string{key});
}

inline bool isStringList(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isStringList(std::string{key});
}

inline bool isSubConfigurationList(const eckit::LocalConfiguration& conf, std::string_view key) {
    return conf.isSubConfigurationList(std::string{key});
}

}  // namespace hacks

// -----------------------------------------------------------------------------
// to_json
// -----------------------------------------------------------------------------
template <>
struct DictToJsonTraits<eckit::LocalConfiguration> {

    static std::string to_json(const eckit::LocalConfiguration& cfg) noexcept(true) {
        try {
            std::ostringstream os;
            os << cfg;
            return os.str();
        }
        catch (...) {
            return "[to_json failed for eckit::LocalConfiguration]";
        }
    }
};

// -----------------------------------------------------------------------------
// DictCreateFromSample / Clone / NeedsChecks
// -----------------------------------------------------------------------------
template <>
struct DictTraits<eckit::LocalConfiguration> {

    static constexpr bool support_checks = false;

    static std::unique_ptr<eckit::LocalConfiguration> make_from_sample_or_throw(std::string_view name) {

        auto cfg = std::make_unique<eckit::LocalConfiguration>();
        cfg->set("SampleName", std::string(name));
        return cfg;
    }

    static std::unique_ptr<eckit::LocalConfiguration> clone_or_throw(const eckit::LocalConfiguration& cfg) {
        return std::make_unique<eckit::LocalConfiguration>(cfg);
    }
};

// -----------------------------------------------------------------------------
// DictHas
// -----------------------------------------------------------------------------
template <>
struct DictHas<eckit::LocalConfiguration> {

    static bool has(const eckit::LocalConfiguration& cfg, std::string_view key) noexcept(false) {
        const std::string k{key};

        try {
            return cfg.has(k);
        }
        catch (const exceptions::Mars2GribGenericException&) {
            throw;
        }
        catch (...) {
            std::throw_with_nested(exceptions::Mars2GribDictException(
                "Internal error while checking presence of key `"s + k + "` in dictionary type `"s +
                    std::string(type_name<eckit::LocalConfiguration>()) + "`",
                Here()));
        }
        __builtin_unreachable();
    }
};

// ============================================================
//  eckit::LocalConfiguration specializations via macros
// ============================================================

//------------------------------------------------------------------------------
// Scalar types
//------------------------------------------------------------------------------

// bool
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(bool, isBoolean, getBool)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(bool, isBoolean, getBool)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(bool, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(bool, set)

// int
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(int, isIntegral, getInt)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(int, isIntegral, getInt)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(int, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(int, set)

// long
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(long, isIntegral, getLong)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(long, isIntegral, getLong)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(long, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(long, set)

// float
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(float, isFloatingPoint, getFloat)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(float, isFloatingPoint, getFloat)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(float, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(float, set)

// double
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(double, isFloatingPoint, getDouble)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(double, isFloatingPoint, getDouble)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(double, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(double, set)

// std::string
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::string, isString, getString)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::string, isString, getString)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::string, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::string, set)

// eckit::LocalConfiguration (sub-configuration)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(eckit::LocalConfiguration, isSubConfiguration, getSubConfiguration)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(eckit::LocalConfiguration, isSubConfiguration, getSubConfiguration)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(eckit::LocalConfiguration, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(eckit::LocalConfiguration, set)


//------------------------------------------------------------------------------
// Vector types
//------------------------------------------------------------------------------


// std::vector<int>
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::vector<int>, isIntegralList, getIntVector)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::vector<int>, isIntegralList, getIntVector)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::vector<int>, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::vector<int>, set)

// std::vector<long>
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::vector<long>, isIntegralList, getLongVector)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::vector<long>, isIntegralList, getLongVector)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::vector<long>, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::vector<long>, set)

// std::vector<float>
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::vector<float>, isFloatingPointList, getFloatVector)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::vector<float>, isFloatingPointList, getFloatVector)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::vector<float>, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::vector<float>, set)

// std::vector<double>
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::vector<double>, isFloatingPointList, getDoubleVector)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::vector<double>, isFloatingPointList, getDoubleVector)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::vector<double>, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::vector<double>, set)

// std::vector<std::string>
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::vector<std::string>, isStringList, getStringVector)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::vector<std::string>, isStringList, getStringVector)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::vector<std::string>, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::vector<std::string>, set)

// std::vector<eckit::LocalConfiguration>
M2G_DEFINE_LOCALCONFIG_DICT_GET_OR_THROW(std::vector<eckit::LocalConfiguration>, isSubConfigurationList,
                                         getSubConfigurations)
M2G_DEFINE_LOCALCONFIG_DICT_GET_OPT(std::vector<eckit::LocalConfiguration>, isSubConfigurationList,
                                    getSubConfigurations)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_THROW(std::vector<eckit::LocalConfiguration>, set)
M2G_DEFINE_LOCALCONFIG_DICT_SET_OR_IGNORE(std::vector<eckit::LocalConfiguration>, set)

}  // namespace metkit::mars2grib::utils::dict_traits

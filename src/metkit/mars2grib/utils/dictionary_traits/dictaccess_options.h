#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "metkit/mars2grib/Options.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/type_traits_name.h"

namespace metkit::mars2grib::utils {

template <>
constexpr std::string_view type_name<metkit::mars2grib::Options>() {
    return "metkit::mars2grib::Options";
}

// -----------------------------------------------------------------------------
// Internal helper to map string keys to Options struct members
// -----------------------------------------------------------------------------
namespace detail {
    inline bool* get_options_ptr(metkit::mars2grib::Options& opt, std::string_view key) {
        if (key == "applyChecks") return &opt.applyChecks;
        if (key == "enableOverride") return &opt.enableOverride;
        if (key == "enableBitsPerValueCompression") return &opt.enableBitsPerValueCompression;
        if (key == "normalizeMars") return &opt.normalizeMars;
        if (key == "normalizeMisc") return &opt.normalizeMisc;
        if (key == "fixMarsGrid") return &opt.fixMarsGrid;
        return nullptr;
    }

    inline const bool* get_options_ptr(const metkit::mars2grib::Options& opt, std::string_view key) {
        if (key == "applyChecks") return &opt.applyChecks;
        if (key == "enableOverride") return &opt.enableOverride;
        if (key == "enableBitsPerValueCompression") return &opt.enableBitsPerValueCompression;
        if (key == "normalizeMars") return &opt.normalizeMars;
        if (key == "normalizeMisc") return &opt.normalizeMisc;
        if (key == "fixMarsGrid") return &opt.fixMarsGrid;
        return nullptr;
    }
} // namespace detail

}  // namespace metkit::mars2grib::utils


namespace metkit::mars2grib::utils::dict_traits {

using std::operator""s;

// -----------------------------------------------------------------------------
// DictTraits
// -----------------------------------------------------------------------------
template <>
struct DictTraits<metkit::mars2grib::Options> {

    static constexpr bool support_checks = true;

    static std::unique_ptr<metkit::mars2grib::Options> make_from_sample_or_throw(std::string_view /*name*/) {
        // Options has no samples, just return default constructed
        return std::make_unique<metkit::mars2grib::Options>();
    }

    static std::unique_ptr<metkit::mars2grib::Options> clone_or_throw(const metkit::mars2grib::Options& h) {
        // Struct has implicit copy constructor
        return std::make_unique<metkit::mars2grib::Options>(h);
    }
};

// -----------------------------------------------------------------------------
// DictHas
// -----------------------------------------------------------------------------
template <>
struct DictHas<metkit::mars2grib::Options> {
    static bool has(const metkit::mars2grib::Options& opt, std::string_view key) noexcept(false) {
        return detail::get_options_ptr(opt, key) != nullptr;
    }
};

// -----------------------------------------------------------------------------
// DictMissing (Options cannot be "missing", they are boolean flags)
// -----------------------------------------------------------------------------
template <>
struct DictMissing<metkit::mars2grib::Options> {
    static bool isMissing(const metkit::mars2grib::Options&, std::string_view) noexcept(false) {
        return false;
    }

    static void setMissing(metkit::mars2grib::Options&, std::string_view key) noexcept(false) {
        throw exceptions::Mars2GribDictException(
            "Cannot set key `"s + std::string(key) + "` to missing in dictionary type `"s +
                std::string(type_name<metkit::mars2grib::Options>()) + "`",
            Here());
    }
};

// -----------------------------------------------------------------------------
// DictToJsonTraits
// -----------------------------------------------------------------------------
template <>
struct DictToJsonTraits<metkit::mars2grib::Options> {
    static std::string to_json(const metkit::mars2grib::Options& opt) {
        return "{\n"
               "  \"applyChecks\": "s + (opt.applyChecks ? "true" : "false") + ",\n" +
               "  \"enableOverride\": "s + (opt.enableOverride ? "true" : "false") + ",\n" +
               "  \"enableBitsPerValueCompression\": "s + (opt.enableBitsPerValueCompression ? "true" : "false") + ",\n" +
               "  \"normalizeMars\": "s + (opt.normalizeMars ? "true" : "false") + ",\n" +
               "  \"normalizeMisc\": "s + (opt.normalizeMisc ? "true" : "false") + ",\n" +
               "  \"fixMarsGrid\": "s + (opt.fixMarsGrid ? "true" : "false") + "\n" +
               "}";
    }

    static void dump_or_ignore(const metkit::mars2grib::Options&, const std::string&) {
        // Not applicable for this simple struct
    }
};

// ============================================================================
// MACROS FOR TYPE MAPPING (Bool, Int, Long)
// ============================================================================

#define M2G_DEFINE_OPTIONS_DICT_ACCESS(CTYPE)                                                          \
    template <>                                                                                        \
    struct DictGetOrThrow<metkit::mars2grib::Options, CTYPE> {                                         \
        static CTYPE get_or_throw(const metkit::mars2grib::Options& opt, std::string_view key) noexcept(false) { \
            if (const bool* ptr = detail::get_options_ptr(opt, key)) {                                 \
                return static_cast<CTYPE>(*ptr);                                                       \
            }                                                                                          \
            throw exceptions::Mars2GribDictException(                                                  \
                "Missing or invalid key `"s + std::string(key) + "` in dictionary `"s +                \
                    std::string(type_name<metkit::mars2grib::Options>()) + "`", Here());               \
        }                                                                                              \
    };                                                                                                 \
                                                                                                       \
    template <>                                                                                        \
    struct DictGetOpt<metkit::mars2grib::Options, CTYPE> {                                             \
        static std::optional<CTYPE> get_opt(const metkit::mars2grib::Options& opt, std::string_view key) noexcept(false) { \
            if (const bool* ptr = detail::get_options_ptr(opt, key)) {                                 \
                return static_cast<CTYPE>(*ptr);                                                       \
            }                                                                                          \
            return std::nullopt;                                                                       \
        }                                                                                              \
    };                                                                                                 \
                                                                                                       \
    template <>                                                                                        \
    struct DictSetOrThrow<metkit::mars2grib::Options, CTYPE> {                                         \
        static void set_or_throw(metkit::mars2grib::Options& opt, std::string_view key, const CTYPE& v) noexcept(false) { \
            if (bool* ptr = detail::get_options_ptr(opt, key)) {                                       \
                *ptr = static_cast<bool>(v);                                                           \
                return;                                                                                \
            }                                                                                          \
            throw exceptions::Mars2GribDictException(                                                  \
                "Unable to set key `"s + std::string(key) + "` in dictionary `"s +                     \
                    std::string(type_name<metkit::mars2grib::Options>()) + "`", Here());               \
        }                                                                                              \
    };                                                                                                 \
                                                                                                       \
    template <>                                                                                        \
    struct DictSetOrIgnore<metkit::mars2grib::Options, CTYPE> {                                        \
        static void set_or_ignore(metkit::mars2grib::Options& opt, std::string_view key, const CTYPE& v) noexcept(false) { \
            if (bool* ptr = detail::get_options_ptr(opt, key)) {                                       \
                *ptr = static_cast<bool>(v);                                                           \
            }                                                                                          \
        }                                                                                              \
    };

// ============================================================================
// APPLY TYPE SPECIALIZATIONS
// ============================================================================

M2G_DEFINE_OPTIONS_DICT_ACCESS(bool)
M2G_DEFINE_OPTIONS_DICT_ACCESS(int)
M2G_DEFINE_OPTIONS_DICT_ACCESS(long)

// String, double, and vector types deliberately left undefined.
// They will fall through to the base traits and throw exceptions::Mars2GribDictException.

}  // namespace metkit::mars2grib::utils::dict_traits
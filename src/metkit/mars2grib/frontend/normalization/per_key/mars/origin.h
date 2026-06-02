/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include <string>
#include <string_view>

#include "eckit/value/Value.h"

#include "metkit/mars2grib/frontend/normalization/per_key/mars/EnumHelper.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Individual sanitization check for the MARS key: origin.
///
/// Behaviour:
/// - If @p in carries @c origin as an integer (e.g. @c 98 for ECMWF),
///   the value is resolved to its canonical string via @ref enum_helper.
/// - If @p in carries @c origin as a string, it is resolved through the
///   same alias->canonical map; case-variants and prefix matches are
///   rejected (strict equality only).
/// - If @p in does not carry @c origin, the canonical default
///   @c "ecmf" is written into @p out.
///
/// The canonical (string) value is always emitted on @p out.
///
template <typename MarsDict_t>
void sanitise_origin_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    static constexpr std::string_view KEY      = "origin";
    static constexpr std::string_view DEFAULT  = "ecmf";

    if (auto asInt = get_opt<long>(in, KEY); asInt.has_value()) {
        set_or_throw<std::string>(out, KEY,
                                  enum_helper::resolve_canonical(language, KEY, *asInt));
        return;
    }

    const std::string raw =
        get_opt<std::string>(in, KEY).value_or(std::string{DEFAULT});
    set_or_throw<std::string>(out, KEY,
                              enum_helper::resolve_canonical(language, KEY, raw));
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

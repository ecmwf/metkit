/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include <string>

#include "eckit/value/Value.h"
#include "metkit/mars2grib/frontend/normalization/per_key/mars/EnumHelper.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Sanitize the MARS key: grid.
///
/// The grid type is [enum, regex, float] with @c uppercase: true in the
/// MARS language. Resolution order:
/// 1. Flat enum alias lookup (e.g. "av", "F128", numeric aliases).
/// 2. Regex match against patterns (^[oOfF][1-9][0-9]*$, etc.).
/// 3. On a regex hit, the value is uppercased.
/// Throws if present but matches neither. No-op if the key is absent.
///
template <typename MarsDict_t>
void sanitise_grid_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    if (auto raw = get_opt<std::string>(in, "grid")) {
        set_or_throw<std::string>(out, "grid",
                                  enum_helper::resolve_canonical_or_regex(language, "grid", *raw));
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

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
/// @brief Sanitize the MARS key: levtype.
///
/// Resolves the user-supplied string value to its canonical form via the
/// MARS language enum alias table. The YAML also allows integer aliases
/// (type: [enum, integer]); integer inputs are stringified and resolved.
/// No-op if the key is absent.
///
template <typename MarsDict_t>
void sanitise_levtype_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    if (auto asInt = get_opt<long>(in, "levtype")) {
        set_or_throw<std::string>(out, "levtype",
                                  enum_helper::resolve_canonical(language, "levtype", *asInt));
        return;
    }
    if (auto raw = get_opt<std::string>(in, "levtype")) {
        set_or_throw<std::string>(out, "levtype",
                                  enum_helper::resolve_canonical(language, "levtype", *raw));
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

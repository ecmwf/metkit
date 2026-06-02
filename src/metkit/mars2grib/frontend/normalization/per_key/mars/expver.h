/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include <string>

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Sanitize the MARS key: expver.
///
/// The expver type is a special "expver" type in the MARS language; values
/// are copied through as-is (no enum alias resolution). No-op if absent.
///
template <typename MarsDict_t>
void sanitise_expver_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    if (auto raw = get_opt<std::string>(in, "expver")) {
        set_or_throw<std::string>(out, "expver", *raw);
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

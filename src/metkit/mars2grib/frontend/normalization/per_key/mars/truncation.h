/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Sanitize the MARS key: truncation.
///
/// Only a native long (or numeric type) is accepted. String values such
/// as \"auto\" and \"none\" are silently ignored (key is absent from
/// scratch). No-op if key is absent.
///
template <typename MarsDict_t>
void sanitise_truncation_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    if (auto v = get_opt<long>(in, "truncation")) {
        set_or_throw<long>(out, "truncation", *v);
    }
    // string variants ("auto", "none") are intentionally not written to out
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

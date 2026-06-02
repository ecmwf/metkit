/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Sanitize the MARS key: date.
///
/// Normalizes to a long integer in YYYYMMDD format. No-op if absent.
///
template <typename MarsDict_t>
void sanitise_date_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    if (auto v = get_opt<long>(in, "date")) {
        set_or_throw<long>(out, "date", *v);
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

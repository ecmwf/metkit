/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Sanitize the MARS key: wavelength.
///
/// The encoder only calls @c has(mars, "wavelength") and never reads the
/// value, but in other branches wavelength is read as a double. The
/// sanitizer therefore writes a double when the key is present. Integer
/// inputs are cast to double. No-op if absent or if the string \"none\"
/// is supplied.
///
template <typename MarsDict_t>
void sanitise_wavelength_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    // Ignore "none" sentinel
    if (auto s = get_opt<std::string>(in, "wavelength")) {
        if (*s == "none") {
            return;
        }
    }
    if (auto v = get_opt<double>(in, "wavelength")) {
        set_or_throw<double>(out, "wavelength", *v);
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

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
/// @brief Sanitize the MARS key: dataset.
///
/// The dataset type is context-dependent in the MARS language; if no flat
/// enum table is available the value is passed through unchanged.
/// No-op if the key is absent.
///
template <typename MarsDict_t>
void sanitise_dataset_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    if (auto raw = get_opt<std::string>(in, "dataset")) {
        set_or_throw<std::string>(out, "dataset",
                                  enum_helper::resolve_canonical(language, "dataset", *raw));
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

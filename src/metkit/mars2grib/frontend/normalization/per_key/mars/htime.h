/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Individual sanitization check for the GRIB key: htime.
///
template <typename MarsDict_t>
void sanitise_htime_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    // TODO: Implement specific validation logic for htime
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

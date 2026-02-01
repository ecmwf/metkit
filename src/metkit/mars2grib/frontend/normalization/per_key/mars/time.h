/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

/**
 * @brief Individual sanitization check for the GRIB key: time.
 */
template <typename MarsDict_t>
void sanitise_time_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language ) {
    // TODO: Implement specific validation logic for time
}

} // namespace metkit::mars2grib::frontend::normalization::per_key

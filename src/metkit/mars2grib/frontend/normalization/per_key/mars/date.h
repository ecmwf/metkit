/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Individual sanitization check for the GRIB key: date.
///
template <typename MarsDict_t>
void sanitise_date_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language) {
    // TODO: Implement specific validation logic for date
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

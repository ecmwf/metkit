/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Individual sanitization check for the GRIB key: level.
///
template <typename T>
void sanitise_level_or_throw(T& value, const eckit::Value& language) {
    // TODO: Implement specific validation logic for level
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/Profiling.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Individual sanitization check for the GRIB key: packing.
///
template <typename MarsDict_t, typename Cntx_t>
void sanitise_packing_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& language, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    // TODO: Implement specific validation logic for packing
    utils::profiling::profileExitFunction(cntx, Here());
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key

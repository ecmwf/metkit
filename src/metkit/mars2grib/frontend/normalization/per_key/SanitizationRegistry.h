/*
 * (C) Copyright 2025- ECMWF
 */

#pragma once

#include <functional>
#include <string>
#include <vector>
#include "metkit/mars2grib/frontend/normalization/per_key/mars/All.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/Profiling.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Registry to provide atomic access to sanitizers for testing.
///
template <typename T, typename Cntx_t>
struct MarsSanitizerRegistry {

    using SanitizerFn = std::function<void(T&, const eckit::Value&, Cntx_t&)>;

    struct Entry {
        std::string key;
        SanitizerFn func;
    };

    static std::vector<Entry> get_all_tests(Cntx_t& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        std::vector<Entry> result{
            {"origin", per_key::sanitise_origin_or_throw<T, Cntx_t>},
            {"class", per_key::sanitise_class_or_throw<T, Cntx_t>},
            {"stream", per_key::sanitise_stream_or_throw<T, Cntx_t>},
            {"type", per_key::sanitise_type_or_throw<T, Cntx_t>},
            {"expver", per_key::sanitise_expver_or_throw<T, Cntx_t>},
            {"date", per_key::sanitise_date_or_throw<T, Cntx_t>},
            {"truncation", per_key::sanitise_truncation_or_throw<T, Cntx_t>}
            // ... the script can be extended to populate this list
        };
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

}  // namespace metkit::mars2grib::frontend::normalization::per_key

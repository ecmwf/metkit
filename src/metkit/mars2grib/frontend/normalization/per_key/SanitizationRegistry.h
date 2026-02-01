/*
 * (C) Copyright 2025- ECMWF
 */

#pragma once

#include <vector>
#include <functional>
#include <string>
#include "metkit/mars2grib/frontend/normalization/per_key/mars/All.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

/**
 * @brief Registry to provide atomic access to sanitizers for testing.
 */
template <typename T>
struct MarsSanitizerRegistry {

    using SanitizerFn = std::function<void(T&, const eckit::Value&)>;

    struct Entry {
        std::string key;
        SanitizerFn func;
    };

    static std::vector<Entry> get_all_tests() {
        return {
            {"origin",     per_key::sanitise_origin_or_throw<T>},
            {"class",      per_key::sanitise_class_or_throw<T>},
            {"stream",     per_key::sanitise_stream_or_throw<T>},
            {"type",       per_key::sanitise_type_or_throw<T>},
            {"expver",     per_key::sanitise_expver_or_throw<T>},
            {"date",       per_key::sanitise_date_or_throw<T>},
            {"truncation", per_key::sanitise_truncation_or_throw<T>}
            // ... the script can be extended to populate this list
        };
    }
};

} // namespace metkit::mars2grib::frontend::normalization::per_key
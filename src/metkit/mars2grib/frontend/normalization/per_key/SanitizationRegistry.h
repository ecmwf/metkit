/*
 * (C) Copyright 2025- ECMWF
 */

#pragma once

#include <functional>
#include <string>
#include <vector>
#include "eckit/value/Value.h"
#include "metkit/mars2grib/frontend/normalization/per_key/mars/All.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

///
/// @brief Registry to provide atomic access to sanitizers for testing.
///
template <typename T>
struct MarsSanitizerRegistry {

    using SanitizerFn = std::function<void(const T&, T&, const eckit::Value&)>;

    struct Entry {
        std::string key;
        SanitizerFn func;
    };

    static std::vector<Entry> get_all_tests() {
        return {
            {"origin", per_key::sanitise_origin_or_throw<T>},
            {"class", per_key::sanitise_class_or_throw<T>},
            {"stream", per_key::sanitise_stream_or_throw<T>},
            {"type", per_key::sanitise_type_or_throw<T>},
            {"expver", per_key::sanitise_expver_or_throw<T>}
            // ... the script can be extended to populate this list
        };
    }

    ///
    /// @brief Run every per-key MARS sanitizer registered above.
    ///
    /// For each registered entry, the dispatcher checks whether the input
    /// dictionary actually carries the key; if so, the corresponding rule is
    /// invoked with @p mars as the read-only source and @p scratch as the
    /// fresh sink. @p scratch is expected to start empty: per-key rules are
    /// the sole writers of normalized values into it.
    ///
    /// @param[in]  mars     Original MARS dictionary (read-only)
    /// @param[out] scratch  Fresh dictionary that rules populate with
    ///                      normalized values
    /// @param[in]  language Language definition driving validation/aliasing
    ///
    static void run_all(const T& mars, T& scratch, const eckit::Value& language) {
        using metkit::mars2grib::utils::dict_traits::DictHas;

        for (const auto& entry : get_all_tests()) {
            if (DictHas<T>::has(mars, entry.key)) {
                entry.func(mars, scratch, language);
            }
        }
    }
};

}  // namespace metkit::mars2grib::frontend::normalization::per_key
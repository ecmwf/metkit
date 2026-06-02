/*
 * (C) Copyright 2025- ECMWF
 */

#pragma once

#include <functional>
#include <string>
#include <vector>
#include "eckit/log/Log.h"
#include "eckit/value/Value.h"
#include "metkit/config/LibMetkit.h"
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
            // Enum string keys
            {"origin",      per_key::sanitise_origin_or_throw<T>},
            {"activity",    per_key::sanitise_activity_or_throw<T>},
            {"class",       per_key::sanitise_class_or_throw<T>},  // must precede model
            {"dataset",     per_key::sanitise_dataset_or_throw<T>},
            {"domain",      per_key::sanitise_domain_or_throw<T>},
            {"experiment",  per_key::sanitise_experiment_or_throw<T>},
            {"levtype",     per_key::sanitise_levtype_or_throw<T>},
            // model depends on class (branch selection); also after country if country is ever added
            {"model",       per_key::sanitise_model_or_throw<T>},
            {"packing",     per_key::sanitise_packing_or_throw<T>},
            {"resolution",  per_key::sanitise_resolution_or_throw<T>},
            {"stattype",    per_key::sanitise_stattype_or_throw<T>},
            {"stream",      per_key::sanitise_stream_or_throw<T>},
            {"type",        per_key::sanitise_type_or_throw<T>},
            // String pass-through / enum+regex
            {"expver",      per_key::sanitise_expver_or_throw<T>},
            {"grid",        per_key::sanitise_grid_or_throw<T>},
            // Numeric long keys (step MUST precede timespan)
            {"anoffset",    per_key::sanitise_anoffset_or_throw<T>},
            {"channel",     per_key::sanitise_channel_or_throw<T>},
            {"chem",        per_key::sanitise_chem_or_throw<T>},
            {"date",        per_key::sanitise_date_or_throw<T>},
            {"direction",   per_key::sanitise_direction_or_throw<T>},
            {"frequency",   per_key::sanitise_frequency_or_throw<T>},
            {"generation",  per_key::sanitise_generation_or_throw<T>},
            {"hdate",       per_key::sanitise_hdate_or_throw<T>},
            {"htime",       per_key::sanitise_htime_or_throw<T>},
            {"ident",       per_key::sanitise_ident_or_throw<T>},
            {"instrument",  per_key::sanitise_instrument_or_throw<T>},
            {"levelist",    per_key::sanitise_levelist_or_throw<T>},
            {"method",      per_key::sanitise_method_or_throw<T>},
            {"number",      per_key::sanitise_number_or_throw<T>},
            {"param",       per_key::sanitise_param_or_throw<T>},
            {"realization", per_key::sanitise_realization_or_throw<T>},
            {"step",        per_key::sanitise_step_or_throw<T>},     // must precede timespan
            {"system",      per_key::sanitise_system_or_throw<T>},
            {"time",        per_key::sanitise_time_or_throw<T>},
            {"truncation",  per_key::sanitise_truncation_or_throw<T>},
            {"timespan",    per_key::sanitise_timespan_or_throw<T>}, // reads step from scratch
            {"wavelength",  per_key::sanitise_wavelength_or_throw<T>},
        };
    }

    ///
    /// @brief Run every per-key MARS sanitizer registered above.
    ///
    /// Each rule is invoked unconditionally with @p mars as the read-only
    /// source and @p scratch as the fresh sink, so that rules carrying a
    /// default value can populate @p scratch even when @p mars omits the
    /// key. @p scratch is expected to start empty: per-key rules are the
    /// sole writers of normalized values into it.
    ///
    /// @param[in]  mars     Original MARS dictionary (read-only)
    /// @param[out] scratch  Fresh dictionary that rules populate with
    ///                      normalized values
    /// @param[in]  language Language definition driving validation/aliasing
    ///
    static void run_all(const T& mars, T& scratch, const eckit::Value& language) {
        std::cout << "mars:    " << metkit::mars2grib::utils::dict_traits::dict_to_json(mars) << std::endl;
        for (const auto& entry : get_all_tests()) {
            entry.func(mars, scratch, language);
        }
        std::cout << "scratch: " << metkit::mars2grib::utils::dict_traits::dict_to_json(scratch) << std::endl;
    }
};

}  // namespace metkit::mars2grib::frontend::normalization::per_key
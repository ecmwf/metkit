#pragma once

// System include
#include <cstddef>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/destine/destineEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t destineMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (!has(mars, "anoffset") && get_or_throw<std::string>(mars, "class") == "d1") {
        if (has(mars, "dataset")) {
            if (get_or_throw<std::string>(mars, "dataset") == "extremes-dt") {
                return static_cast<std::size_t>(DestineType::ExtremesDT);
            }
            else if (get_or_throw<std::string>(mars, "dataset") == "climate-dt") {
                return static_cast<std::size_t>(DestineType::ClimateDT);
            }
            else {
                /// @todo Consider throwing an exception here instead, since an unrecognized dataset value likely
                /// indicates a user error or unsupported case, rather than a simple "not applicable" scenario. The
                /// current approach of returning MISSING may lead to silent failures or harder-to-debug issues
                /// downstream.
                return compile_time_registry_engine::MISSING;
            }
        }
        else {
            /// @todo Consider throwing an exception here instead, since the absence of the "dataset" key likely
            /// indicates a user error or incomplete input, rather than a simple "not applicable" scenario. The current
            /// approach of returning MISSING may lead to silent failures or harder-to-debug issues downstream.
            return compile_time_registry_engine::MISSING;
        }
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

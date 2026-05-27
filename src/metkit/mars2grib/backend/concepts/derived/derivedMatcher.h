#pragma once

// System include
#include <cstddef>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/derived/derivedEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t derivedMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    const auto& type = get_or_throw<std::string>(mars, "type");
    if (type == "es" ||    // Ensemble standard deviation
        type == "ses" ||   // Ensemble spread of estimation
        type == "taem" ||  // Time-averaged ensemble mean
        type == "taes" ||  // Time-averaged ensemble standard deviation
        type == "efi" ||   // Extreme forecast index
        type == "sot"      // Shift of tails
    ) {
        return static_cast<std::size_t>(DerivedType::Default);
    }

    if (type == "em") {  // Ensemble mean (special handling for brightness temperature products)
        if (has(mars, "channel") && has(mars, "param") && get_or_throw<long>(mars, "param") == 194 &&
            has(mars, "stream") && get_or_throw<std::string>(mars, "stream") == "elda") {
            return static_cast<std::size_t>(DerivedType::BrightnessTemperatureEnsembleMean);
        }
        else {
            return static_cast<std::size_t>(DerivedType::Default);
        }
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/mars2grib/backend/concepts/satellite/satelliteEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t satelliteMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    try {

        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        // Default satellite: requires full satellite identification keys
        if (has(mars, "channel") && has(mars, "ident") && has(mars, "instrument")) {
            return static_cast<std::size_t>(SatelliteType::Default);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `satellite` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

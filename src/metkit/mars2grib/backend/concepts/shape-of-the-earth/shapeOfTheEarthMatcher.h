#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shapeOfTheEarthEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t shapeOfTheEarthMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {

        using metkit::mars2grib::utils::dict_traits::has;

        // NOTE: Spherical harmonics is encoded without shape of the earth
        if (has(mars, "truncation")) {
            return compile_time_registry_engine::MISSING;
        }

        return static_cast<std::size_t>(ShapeOfTheEarthType::Default);
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `shapeOfTheEarth` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

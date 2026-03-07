#pragma once

// System include
#include <cstddef>

// Utils
#include "eckit/geo/Grid.h"
#include "eckit/spec/Custom.h"
#include "metkit/mars2grib/backend/concepts/representation/representationEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t representationMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(mars, "truncation")) {
        return static_cast<std::size_t>(RepresentationType::SphericalHarmonics);
    }

    const auto marsGrid = get_or_throw<std::string>(mars, "grid");
    const auto gridType = eckit::geo::GridFactory::build(eckit::spec::Custom{{"grid", marsGrid}})->type();
    if (gridType == "regular-gg") {
        return static_cast<std::size_t>(RepresentationType::RegularGaussian);
    }
    else if (gridType == "reduced-gg") {
        return static_cast<std::size_t>(RepresentationType::ReducedGaussian);
    }
    else if (gridType == "regular-ll") {
        return static_cast<std::size_t>(RepresentationType::Latlon);
    }

    throw utils::exceptions::Mars2GribMatcherException(
        "Cannot match grid \"" + marsGrid + "\" with grid type \"" + gridType + "\"! ", Here());
}

}  // namespace metkit::mars2grib::backend::concepts_

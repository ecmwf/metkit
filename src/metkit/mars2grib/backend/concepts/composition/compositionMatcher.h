#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/mars2grib/backend/concepts/composition/compositionEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t compositionMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::utils::dict_traits::has;

        if (has(mars, "chem")) {
            return static_cast<std::size_t>(CompositionType::Chem);
        }

        if (has(mars, "wavelength")) {
            return static_cast<std::size_t>(CompositionType::Aerosol);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `composition` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

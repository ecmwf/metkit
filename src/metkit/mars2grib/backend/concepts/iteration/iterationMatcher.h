#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/iteration/iterationEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t iterationMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(mars, "iteration")) {
        return static_cast<size_t>(IterationType::Default);
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

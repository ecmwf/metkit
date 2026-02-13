#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t referenceTimeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(mars, "hdate")) {
        return static_cast<size_t>(ReferenceTimeType::Reforecast);
    }
    return static_cast<size_t>(ReferenceTimeType::Standard);
}

}  // namespace metkit::mars2grib::backend::concepts_

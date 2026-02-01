#pragma once

// System include
#include <cstddef>
#include <iostream>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t referenceTimeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    std::cout << " - referenceTime matcher" << std::endl;

    return 9999999;
}

}  // namespace metkit::mars2grib::backend::concepts_

#pragma once

// System include
#include <cstddef>
#include <iostream>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t packingMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    std::cout << " - packing matcher" << std::endl;

    return 9999999;
}

}  // namespace metkit::mars2grib::backend::concepts_

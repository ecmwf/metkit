#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <eckit/types/DateTime.h>
#include "eckit/exception/Exceptions.h"

namespace metkit::mars2grib::utils::wave {

struct WaveSpectraInfo {
    long numDirections;
    long scaleFactorDirections;
    std::vector<long> scaledValuesDirections;

    long numFrequencies;
    long scaleFactorFrequencies;
    std::vector<long> scaledValuesFrequencies;
};

}  // namespace metkit::mars2grib::utils::wave

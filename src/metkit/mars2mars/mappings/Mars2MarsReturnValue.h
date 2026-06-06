#pragma once

#include "eckit/config/LocalConfiguration.h"

namespace metkit::mars2mars {

template <typename MarsDict>
struct Mars2MarsResult {
    MarsDict mars;
    eckit::LocalConfiguration misc;
};

} // namespace metkit::mars2mars
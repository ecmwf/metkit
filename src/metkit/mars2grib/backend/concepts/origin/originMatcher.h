#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/mars2grib/backend/concepts/origin/originEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t originMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        return static_cast<std::size_t>(OriginType::Default);
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `origin` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

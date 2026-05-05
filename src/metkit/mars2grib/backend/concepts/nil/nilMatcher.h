#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/mars2grib/backend/concepts/nil/nilEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t nilMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        return static_cast<std::size_t>(NilType::Default);
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException("Unable to match `nil` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

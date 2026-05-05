#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrangeEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t longrangeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::utils::dict_traits::has;

        if (has(mars, "method") && has(mars, "system")) {
            return static_cast<size_t>(LongrangeType::Default);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `longrange` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

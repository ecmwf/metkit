#pragma once

// System include
#include <cstddef>
#include <string>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/model-error/modelErrorEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t modelErrorMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    // Concept does not apply unless "type" is present and equals "eme"
    if (!has(mars, "type") || get_or_throw<std::string>(mars, "type") != "eme") {
        return compile_time_registry_engine::MISSING;
    }

    // At this point the request is a model-error request: "number" is mandatory
    if (!has(mars, "number")) {
        throw utils::exceptions::Mars2GribMatcherException(
            "modelError concept requires MARS key \"number\" when type=\"eme\"", Here());
    }

    return static_cast<std::size_t>(ModelErrorType::Default);
}

}  // namespace metkit::mars2grib::backend::concepts_

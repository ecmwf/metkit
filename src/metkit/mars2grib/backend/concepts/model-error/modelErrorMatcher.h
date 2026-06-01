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


    try {

        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        if (!has(mars, "type") ){
            throw metkit::mars2grib::utils::exceptions::Mars2GribMatcherException(
                "MARS key `type` is required to determine applicability of the `modelError` concept but is missing. This is a contract violation by the upstream tool that populates the MARS dictionary.",
                Here());
        }

        // Concept does not apply unless "type" is present and equals "eme"
        if ( (get_or_throw<std::string>(mars, "type") != "eme" &&
            get_or_throw<std::string>(mars, "type") != "me")) {
            return compile_time_registry_engine::MISSING;
        }

        // At this point the request is a model-error request: "number" is mandatory
        if (has(mars, "number")) {
            return static_cast<std::size_t>(ModelErrorType::ComponentIndex);
        }

        if ( has(mars,"coeffindex") ) {
            return static_cast<std::size_t>(ModelErrorType::FourierCoefficients);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        // Rethrow nested exceptions with a more specific message
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribMatcherException(
                "An error occurred while matching the `modelError` concept. Check nested exception for details.", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

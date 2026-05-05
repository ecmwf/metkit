#pragma once

// System include
#include <cstddef>
#include <exception>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/ensemble/ensembleEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t ensembleMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    // Skip model-error products: in that case "number" identifies the
    // model-error realization, not an ensemble member.
    if (has(mars, "type") && (get_or_throw<std::string>(mars, "type") == "eme" || get_or_throw<std::string>(mars, "type") == "me")) {
        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `ensemble` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_

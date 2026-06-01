#pragma once

// System include
#include <cstddef>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/ensemble/ensembleEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t ensembleMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    // Skip model-error products: in that case "number" identifies the
    // model-error realization, not an ensemble member.
    if (has(mars, "type") && (get_or_throw<std::string>(mars, "type") == "eme" || get_or_throw<std::string>(mars, "type") == "me")) {
        return compile_time_registry_engine::MISSING;
    }

    if (has(mars, "number")) {
        return static_cast<std::size_t>(EnsembleType::Individual);
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

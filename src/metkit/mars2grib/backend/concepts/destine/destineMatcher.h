#pragma once

// System include
#include <cstddef>
#include <string>

// Project includes
#include "metkit/mars2grib/backend/concepts/destine/destineEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t destineMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (!has(mars, "anoffset") && get_or_throw<std::string>(mars, "class") == "d1") {
        if (has(mars, "dataset")) {
            if (get_or_throw<std::string>(mars, "dataset") == "extremes-dt") {
                return static_cast<std::size_t>(DestineType::ExtremesDT);
            }
            else if (get_or_throw<std::string>(mars, "dataset") == "climate-dt") {
                return static_cast<std::size_t>(DestineType::ClimateDT);
            }
            else {
                throw Mars2GribGenericException{"Unknown value \"" + get_or_throw<std::string>(mars, "dataset") +
                                                "\" for mars keyword \"dataset\"!"};
            }
        }
        else {
            throw Mars2GribGenericException{
                "Missing required mars keyword \"dataset\" for class \"d1\" without \"anoffset\"!"};
        }
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

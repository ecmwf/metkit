#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractParam(const std::string& keyword,
                  const metkit::codes::CodesHandle& grib,
                  MarsDict& mars,
                  MiscDict&) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has("paramId")) {
            throw Grib2MarsGenericException(
                "GRIB message is missing required key `paramId` for MARS keyword `" + keyword + "`",
                Here());
        }

        const long value = grib.getLong("paramId");
        set_or_throw<long>(mars, keyword, value);
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException(
                "Failed to extract MARS keyword `" + keyword + "`",
                Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractMisc(const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)mars;

        if (grib.has("generatingProcessIdentifier")) {
            const auto generatingProcessIdentifier = grib.getLong("generatingProcessIdentifier");
            misc.set("generatingProcessIdentifier", generatingProcessIdentifier);
        }

        if (grib.getString("type") != "ai" && grib.has("typeOfProcessedData")) {
            const auto typeOfProcessedData = grib.getString("typeOfProcessedData");
            if (typeOfProcessedData != "missing") {
                misc.set("typeOfProcessedData", typeOfProcessedData);
            }
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MISC keywords", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
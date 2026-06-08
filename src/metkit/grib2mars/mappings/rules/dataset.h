#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractDataset(const std::string& keyword,
                    const metkit::codes::CodesHandle& grib,
                    MarsDict& mars,
                    MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)misc;

        if (!grib.has(keyword)) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `" + keyword +
                    "` required to extract MARS keyword `" + keyword + "`",
                Here());
        }

        const std::string value = grib.getString(keyword);

        set_or_throw<std::string>(mars, keyword, value);
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException(
                "Failed to extract MARS keyword `" + keyword + "`",
                Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
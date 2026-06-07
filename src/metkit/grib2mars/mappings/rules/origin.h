#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractOrigin(const std::string& keyword,
                   const metkit::codes::CodesHandle& grib,
                   MarsDict& mars,
                   MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)misc;

        if (grib.has("origin")) {
            const std::string value = grib.getString("origin");
            set_or_throw<std::string>(mars, keyword, value);
            return;
        }

        if (grib.has("centre")) {
            const std::string value = grib.getString("centre");
            set_or_throw<std::string>(mars, keyword, value);
            return;
        }

        throw Grib2MarsGenericException(
            "Missing GRIB keys `origin` and `centre` required to extract "
            "MARS keyword `" + keyword + "`",
            Here());
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException(
                "Failed to extract MARS keyword `" + keyword + "`",
                Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
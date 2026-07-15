#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractTime(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)misc;

        if (!grib.has("hour")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `hour` required to extract MARS keyword `" + keyword + "`", Here());
        }

        if (!grib.has("minute")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `minute` required to extract MARS keyword `" + keyword + "`", Here());
        }

        if (!grib.has("second")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `second` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const long hour   = grib.getLong("hour");
        const long minute = grib.getLong("minute");
        const long second = grib.getLong("second");

        const long value = hour * 10000 + minute * 100 + second;

        set_or_throw<long>(mars, keyword, value);
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
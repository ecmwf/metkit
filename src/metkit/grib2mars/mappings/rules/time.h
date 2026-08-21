#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractTime(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                 const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        (void)misc;

        if (!grib.has("hour")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `hour` required to extract MARS keyword `" + keyword + "`", Here());
        }

        if (!grib.has("minute")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `minute` required to extract MARS keyword `" + keyword + "`", Here());
        }

        if (grib.has("second") && grib.getLong("second") != 0) {
            throw Grib2MarsGenericException("Non-zero GRIB key `second` is not supported!", Here());
        }

        const long hour   = grib.getLong("hour");
        const long minute = grib.getLong("minute");

        const long value = hour * 100 + minute;

        set_or_throw<long>(mars, keyword, value);
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
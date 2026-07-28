#pragma once

#include <optional>
#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractLevelist(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                     MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)misc;

        if (!grib.has("levtype")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `levtype` required to extract MARS keyword `" + keyword + "`", Here());
        }

        if (!grib.has("level")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `level` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const std::string levtype = grib.getString("levtype");

        std::optional<long> levelist;

        if (levtype == "pl") {
            if (!grib.has("pressureUnits")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `pressureUnits` required to extract "
                    "pressure-level MARS keyword `" +
                        keyword + "`",
                    Here());
            }

            const std::string pressureUnits = grib.getString("pressureUnits");

            if (pressureUnits == "hPa") {
                const long level = grib.getLong("level");
                levelist         = level * 100;
            }
        }

        if (!levelist) {
            const long level = grib.getLong("level");
            levelist         = level;
        }

        set_or_throw<long>(mars, keyword, *levelist);
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
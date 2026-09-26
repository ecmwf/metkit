#pragma once

#include <optional>
#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractLevelist(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                     const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
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

        if (levtype == "pl") {
            if (!grib.has("pressureUnits")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `pressureUnits` required to extract "
                    "pressure-level MARS keyword `" +
                        keyword + "`",
                    Here());
            }

            const auto pressureUnits = grib.getString("pressureUnits");
            auto level               = grib.getDouble("level");

            if (pressureUnits == "Pa") {
                level /= 100;  // Convert from Pa to hPa
            }

            set_or_throw<double>(mars, keyword, level);
        }
        else {
            const auto level = grib.getLong("level");
            set_or_throw<long>(mars, keyword, level);
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
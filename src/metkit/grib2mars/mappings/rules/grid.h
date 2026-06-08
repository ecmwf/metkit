#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractGrid(const std::string& keyword,
                 const metkit::codes::CodesHandle& grib,
                 MarsDict& mars,
                 MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)misc;

        if (!grib.has("gridType")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `gridType` required to extract MARS keyword `" +
                    keyword + "`",
                Here());
        }

        const std::string gridType = grib.getString("gridType");

        if (gridType == "sh") {
            throw Grib2MarsGenericException(
                "Cannot extract MARS keyword `grid` from spherical harmonics "
                "GRIB message; `truncation` is required instead",
                Here());
        }

        if (gridType == "regular_ll") {
            if (!grib.has("iDirectionIncrementInDegrees")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `iDirectionIncrementInDegrees` required "
                    "to extract regular_ll MARS grid",
                    Here());
            }

            if (!grib.has("jDirectionIncrementInDegrees")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `jDirectionIncrementInDegrees` required "
                    "to extract regular_ll MARS grid",
                    Here());
            }

            const double dx = grib.getDouble("iDirectionIncrementInDegrees");
            const double dy = grib.getDouble("jDirectionIncrementInDegrees");

            const std::string grid =
                std::to_string(dx) + "/" + std::to_string(dy);

            set_or_throw<std::string>(mars, keyword, grid);

            return;
        }

        if (!grib.has("gridName")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `gridName` required to extract MARS keyword `" +
                    keyword + "` for gridType `" + gridType + "`",
                Here());
        }

        const std::string gridName = grib.getString("gridName");

        set_or_throw<std::string>(mars, keyword, gridName);
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException(
                "Failed to extract MARS keyword `" + keyword + "`",
                Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
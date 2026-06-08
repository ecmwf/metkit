#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractTruncation(const std::string& keyword,
                       const metkit::codes::CodesHandle& grib,
                       MarsDict& mars,
                       MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has("gridType")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `gridType` required to extract MARS keyword `" +
                    keyword + "`",
                Here());
        }

        const std::string gridType = grib.getString("gridType");

        if (gridType != "sh") {
            throw Grib2MarsGenericException(
                "Cannot extract MARS keyword `truncation` from non-spherical "
                "harmonics gridType `" +
                    gridType + "`",
                Here());
        }

        if (!grib.has("J")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `J` required to extract MARS keyword `" +
                    keyword + "`",
                Here());
        }

        const long truncation = grib.getLong("J");

        set_or_throw<long>(mars, keyword, truncation);

        if (grib.has("laplacianOperator")) {
            const long laplacianOperator = grib.getLong("laplacianOperator");

            misc.set("laplacianOperator", laplacianOperator);
        }
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException(
                "Failed to extract MARS keyword `" + keyword + "`",
                Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
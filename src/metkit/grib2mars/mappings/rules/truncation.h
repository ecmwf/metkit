#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractTruncation(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                       MiscDict& misc, const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        if (!grib.has("gridType")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `gridType` required to extract MARS keyword `" + keyword + "`", Here());
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
            throw Grib2MarsGenericException("Missing GRIB key `J` required to extract MARS keyword `" + keyword + "`",
                                            Here());
        }

        const long j = grib.getLong("J");
        const long k = grib.getLong("K");
        const long m = grib.getLong("M");

        if (j != k || j != m) {
            throw Grib2MarsGenericException("Grib keys `J/K/M` must be equal! J=" + std::to_string(j) +
                                                ", K=" + std::to_string(k) + ", M=" + std::to_string(m),
                                            Here());
        }

        // Set MARS truncation based on J, which is equal to K and M
        set_or_throw<long>(mars, keyword, j);

        if (grib.has("JS")) {
            const long js = grib.getLong("JS");
            const long ks = grib.getLong("KS");
            const long ms = grib.getLong("MS");

            if (js != ks || js != ms) {
                throw Grib2MarsGenericException("Grib keys `JS/KS/MS` must be equal! JS=" + std::to_string(js) +
                                                    ", KS=" + std::to_string(ks) + ", MS=" + std::to_string(ms),
                                                Here());
            }

            if (js > 0 && js <= j) {
                // Set subSetTruncation based on JS, which is equal to KS and MS
                set_or_throw<long>(misc, "subSetTruncation", js);
            }
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
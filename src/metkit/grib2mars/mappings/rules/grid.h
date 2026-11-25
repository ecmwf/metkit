#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractGrid(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                 const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::get_or_throw;
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has("gridType")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `gridType` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const std::string gridType = grib.getString("gridType");

        if (get_or_throw<bool>(opts, "skipSection3")) {
            if (!grib.has("gridSpec")) {
                throw Grib2MarsGenericException("Missing GRIB key `gridSpec` required to extract MARS keyword `grid`",
                                                Here());
            }

            const auto gridSpec = grib.getString("gridSpec");
            set_or_throw<std::string>(mars, "grid", gridSpec);

            if (gridType == "sh") {
                const long j = grib.getLong("J");
                const long k = grib.getLong("K");
                const long m = grib.getLong("M");

                if (j != k || j != m) {
                    throw Grib2MarsGenericException("Grib keys `J/K/M` must be equal! J=" + std::to_string(j) +
                                                        ", K=" + std::to_string(k) + ", M=" + std::to_string(m),
                                                    Here());
                }

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

            return;
        }


        if (gridType == "sh") {
            const long j = grib.getLong("J");
            const long k = grib.getLong("K");
            const long m = grib.getLong("M");

            if (j != k || j != m) {
                throw Grib2MarsGenericException("Grib keys `J/K/M` must be equal! J=" + std::to_string(j) +
                                                    ", K=" + std::to_string(k) + ", M=" + std::to_string(m),
                                                Here());
            }

            // Set MARS grid based on J, which is equal to K and M
            const std::string grid = "T" + std::to_string(j);
            set_or_throw<std::string>(mars, keyword, grid);

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

            return;
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

            const std::string grid = std::to_string(dx) + "/" + std::to_string(dy);

            set_or_throw<std::string>(mars, keyword, grid);

            return;
        }

        if (!grib.has("gridName")) {
            throw Grib2MarsGenericException("Missing GRIB key `gridName` required to extract MARS keyword `" + keyword +
                                                "` for gridType `" + gridType + "`",
                                            Here());
        }

        const std::string gridName = grib.getString("gridName");
        set_or_throw<std::string>(mars, keyword, gridName);
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl

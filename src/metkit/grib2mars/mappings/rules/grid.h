#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

///
/// @brief Validate the truncation keys of a spherical harmonics message.
///
/// Checks that `J/K/M` are equal and, when present, that `JS/KS/MS` are equal. A valid subset
/// truncation (`0 < JS <= J`) is stored as `subSetTruncation` in the misc dictionary.
///
/// @return The truncation `J`, which is equal to `K` and `M`
///
template <class MiscDict>
long extractSphericalHarmonicsTruncation(const metkit::codes::CodesHandle& grib, MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    // Read three GRIB keys that must hold the same value
    const auto getEqual = [&grib](const std::string& a, const std::string& b, const std::string& c) {
        const long va = grib.getLong(a);
        const long vb = grib.getLong(b);
        const long vc = grib.getLong(c);

        if (va != vb || va != vc) {
            throw Grib2MarsGenericException("Grib keys `" + a + "/" + b + "/" + c + "` must be equal! " + a + "=" +
                                                std::to_string(va) + ", " + b + "=" + std::to_string(vb) + ", " + c +
                                                "=" + std::to_string(vc),
                                            Here());
        }

        return va;
    };

    const long j = getEqual("J", "K", "M");

    if (grib.has("JS")) {
        const long js = getEqual("JS", "KS", "MS");

        if (js > 0 && js <= j) {
            // Set subSetTruncation based on JS, which is equal to KS and MS
            set_or_throw<long>(misc, "subSetTruncation", js);
        }
    }

    return j;
}

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
                extractSphericalHarmonicsTruncation(grib, misc);
            }

            return;
        }

        if (gridType == "sh") {
            // Set MARS grid based on J, which is equal to K and M
            const long j           = extractSphericalHarmonicsTruncation(grib, misc);
            const std::string grid = "T" + std::to_string(j);
            set_or_throw<std::string>(mars, keyword, grid);

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

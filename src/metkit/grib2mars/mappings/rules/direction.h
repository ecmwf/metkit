#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractDirection(const std::string& keyword,
                      const metkit::codes::CodesHandle& grib,
                      MarsDict& mars,
                      MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has(keyword)) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `" + keyword +
                    "` required to extract MARS keyword `" + keyword + "`",
                Here());
        }

        const long direction = grib.getLong(keyword);

        set_or_throw<long>(mars, keyword, direction);

        if (grib.has("scaledValuesOfWaveDirections")) {
            constexpr double rad2deg =
                57.29577951308232087679815481410517033240547246656432154916;

            std::vector<double> waveDirections =
                grib.getDoubleArray("scaledValuesOfWaveDirections");

            double directionScalingFactor = 0.0;

            if (grib.has("scaleFactorOfWaveDirections")) {
                const double scaleFactorOfWaveDirections =
                    grib.getDouble("scaleFactorOfWaveDirections");

                directionScalingFactor =
                    std::pow(10.0, scaleFactorOfWaveDirections);
            }
            else {
                if (!grib.has("directionScalingFactor")) {
                    throw Grib2MarsGenericException(
                        "Missing both `scaleFactorOfWaveDirections` and "
                        "`directionScalingFactor` while extracting MARS keyword `" +
                            keyword + "`",
                        Here());
                }

                directionScalingFactor =
                    grib.getDouble("directionScalingFactor");
            }

            std::for_each(
                waveDirections.begin(),
                waveDirections.end(),
                [directionScalingFactor, rad2deg](double& value) {
                    value /= directionScalingFactor * rad2deg;
                });

            const double scaleFactorOfWaveDirections =
                std::log10(directionScalingFactor);

            misc.set("scaleFactorOfWaveDirections", scaleFactorOfWaveDirections);
            misc.set("waveDirections", waveDirections);
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
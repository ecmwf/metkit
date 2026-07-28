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
void extractFrequency(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                      MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has(keyword)) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `" + keyword + "` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const long frequency = grib.getLong(keyword);

        set_or_throw<long>(mars, keyword, frequency);

        if (grib.has("scaledValuesOfWaveFrequencies")) {
            std::vector<double> waveFrequencies = grib.getDoubleArray("scaledValuesOfWaveFrequencies");

            double frequencyScalingFactor = 0.0;

            if (grib.has("scaleFactorOfWaveFrequencies")) {
                const double scaleFactorOfWaveFrequencies = grib.getDouble("scaleFactorOfWaveFrequencies");

                frequencyScalingFactor = std::pow(10.0, scaleFactorOfWaveFrequencies);
            }
            else {
                if (!grib.has("frequencyScalingFactor")) {
                    throw Grib2MarsGenericException(
                        "Missing both `scaleFactorOfWaveFrequencies` and "
                        "`frequencyScalingFactor` while extracting MARS keyword `" +
                            keyword + "`",
                        Here());
                }

                frequencyScalingFactor = grib.getDouble("frequencyScalingFactor");
            }

            std::for_each(waveFrequencies.begin(), waveFrequencies.end(),
                          [frequencyScalingFactor](double& value) { value /= frequencyScalingFactor; });

            const double scaleFactorOfWaveFrequencies = std::log10(frequencyScalingFactor);

            misc.set("scaleFactorOfWaveFrequencies", scaleFactorOfWaveFrequencies);
            misc.set("waveFrequencies", waveFrequencies);
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
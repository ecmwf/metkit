#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractInstrument(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                       MiscDict& misc, const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        if (!grib.has(keyword)) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `" + keyword + "` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const long value = grib.getLong(keyword);

        set_or_throw<long>(mars, keyword, value);

        if (grib.has("satelliteSeries")) {
            const long satelliteSeries = grib.getLong("satelliteSeries");
            misc.set("satelliteSeries", satelliteSeries);
        }

        if (grib.has("scaleFactorOfCentralWaveNumber")) {
            const long scaleFactorOfCentralWaveNumber = grib.getLong("scaleFactorOfCentralWaveNumber");
            misc.set("scaleFactorOfCentralWaveNumber", scaleFactorOfCentralWaveNumber);
        }

        if (grib.has("scaledValueOfCentralWaveNumber")) {
            const long scaledValueOfCentralWaveNumber = grib.getLong("scaledValueOfCentralWaveNumber");
            misc.set("scaledValueOfCentralWaveNumber", scaledValueOfCentralWaveNumber);
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
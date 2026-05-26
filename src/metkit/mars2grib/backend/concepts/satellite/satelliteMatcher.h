#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/satellite/satelliteEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t satelliteMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    // BrightnessTemperature (paramId=194): only requires channel.
    // Section 2 (local def 37) encodes channelNumber + numberOfFrequencies.
    // Section 4 satellite band metadata (ident, instrument, series, waveNumber)
    // is only present for PDT 32/33 — the encoding handles this conditionally.
    if (has(mars, "channel") && has(mars, "param") && get_or_throw<long>(mars, "param") == 194 && has(mars, "stream") &&
        get_or_throw<std::string>(mars, "stream") == "oper") {
        return static_cast<std::size_t>(SatelliteType::BrightnessTemperature);
    }

    // Default satellite: requires full satellite identification keys
    if (has(mars, "channel") && has(mars, "ident") && has(mars, "instrument")) {
        return static_cast<std::size_t>(SatelliteType::Default);
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

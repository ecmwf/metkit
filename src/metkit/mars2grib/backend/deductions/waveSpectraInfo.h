#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/mars2grib/utils/waveUtils.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

using namespace metkit::mars2grib::utils::wave;

namespace metkit::mars2grib::backend::deductions {


template <class OptDict_t, class MarsDict_t, class ParDict_t>
WaveSpectraInfo waveSpectraInfo_or_throw(const OptDict_t& opt, const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        WaveSpectraInfo out{};

        // Not a good idea to put this in the parametrization. In ECMWF we should always encode
        // spectra wit scale factors of 2 and 6 for directions and frequencies respectively.
        // Giving the user the possibility to override this via par.* is just exposing the possibility
        // to create invalid GRIB files!!!!
        long scaleFactorOfWaveDirections  = get_opt<long>(par, "scaleFactorOfWaveDirections").value_or(2L);
        long scaleFactorOfWaveFrequencies = get_opt<long>(par, "scaleFactorOfWaveFrequencies").value_or(6L);

        // Directions
        bool hasNumberOfWaveDirections = has(par, "numberOfWaveDirections");
        bool hasWaveDirections         = has(par, "waveDirections");

        if (hasNumberOfWaveDirections && !hasWaveDirections) {
            throw Mars2GribDeductionException(
                "Not implemented! Getting wave directions from numberOfWaveDirections is still not supported", Here());
        }
        else if (!hasNumberOfWaveDirections && hasWaveDirections) {
            std::vector<double> waveDirections = get_or_throw<std::vector<double>>(par, "waveDirections");
            out.numDirections                  = static_cast<long>(waveDirections.size());
            out.scaleFactorDirections          = scaleFactorOfWaveDirections;
            out.scaledValuesDirections.resize(waveDirections.size());
            for (std::size_t i = 0; i < waveDirections.size(); ++i) {
                out.scaledValuesDirections[i] =
                    static_cast<long>(std::round(waveDirections[i] * std::pow(10.0, scaleFactorOfWaveDirections)));
            }
        }
        else if (hasNumberOfWaveDirections && hasWaveDirections) {
            throw Mars2GribDeductionException(
                "Inconsistent Mars/Par dictionaries: both numberOfWaveDirections and waveDirections are present",
                Here());
        }
        else {
            throw Mars2GribDeductionException(
                "Insufficient Mars/Par dictionaries: neither numberOfWaveDirections nor waveDirections are present",
                Here());
        }

        // Frequencies
        bool hasNumberOfWaveFrequencies = has(par, "numberOfWaveFrequencies");
        bool hasWaveFrequencies         = has(par, "waveFrequencies");

        if (hasNumberOfWaveFrequencies && !hasWaveFrequencies) {
            throw Mars2GribDeductionException(
                "Not implemented! Getting wave frequencies from numberOfWaveFrequencies is still not supported",
                Here());
        }
        else if (!hasNumberOfWaveFrequencies && hasWaveFrequencies) {
            std::vector<double> waveFrequencies = get_or_throw<std::vector<double>>(par, "waveFrequencies");
            out.numFrequencies                  = static_cast<long>(waveFrequencies.size());
            out.scaleFactorFrequencies          = scaleFactorOfWaveFrequencies;
            out.scaledValuesFrequencies.resize(waveFrequencies.size());
            for (std::size_t i = 0; i < waveFrequencies.size(); ++i) {
                out.scaledValuesFrequencies[i] =
                    static_cast<long>(std::round(waveFrequencies[i] * std::pow(10.0, scaleFactorOfWaveFrequencies)));
            }
        }
        else if (hasNumberOfWaveFrequencies && hasWaveFrequencies) {
            throw Mars2GribDeductionException(
                "Inconsistent Mars/Par dictionaries: both numberOfWaveFrequencies and waveFrequencies are present",
                Here());
        }
        else {
            throw Mars2GribDeductionException(
                "Insufficient Mars/Par dictionaries: neither numberOfWaveFrequencies nor waveFrequencies are present",
                Here());
        }


        return out;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get wave spectra information from Mars and Par dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

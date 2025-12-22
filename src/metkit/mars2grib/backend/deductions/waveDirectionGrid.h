/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Metadata and scaled representation of a wave direction grid.
 *
 * This structure describes a discretized wave direction grid together with
 * its scaled integer representation. It is intended for use in contexts where
 * wave propagation directions must be stored, transmitted, or encoded as
 * integers (e.g. GRIB encoding), while preserving a clear mapping to physical
 * directional angles expressed in radians.
 *
 * The scaling convention is logarithmic and based on powers of ten:
 * - `scaleFactorDirections` represents the base-10 logarithm of the real
 *   scaling factor applied to the physical direction values.
 * - For example, a value of `scaleFactorDirections = 2` corresponds to a
 *   real scaling factor of \f$10^2\f$.
 *
 * Scaled integer values are obtained as:
 * \f[
 *   \text{scaledValue}_i =
 *     \mathrm{round}\!\left( \theta_i \times 10^{\text{scaleFactorDirections}} \right)
 * \f]
 * where \f$\theta_i\f$ is the physical wave direction expressed in radians.
 *
 * The inverse operation, used to reconstruct physical directions, is:
 * \f[
 *   \theta_i =
 *     \frac{\text{scaledValue}_i}{10^{\text{scaleFactorDirections}}}
 * \f]
 *
 * This structure is a plain data container and does not enforce internal
 * consistency between the number of directions and the size of the associated
 * value vector. Such validation is expected to be performed by the caller or
 * by higher-level deduction logic.
 *
 * @note
 *   The directional angles are assumed to span the interval \f$[0, 2\pi)\f$
 *   and to follow the discretization conventions of spectral wave models.
 */
struct WaveDirectionGrid {

    /**
     * @brief Number of discrete wave directions in the grid.
     */
    long numDirections;

    /**
     * @brief Base-10 logarithm of the real direction scaling factor.
     *
     * This value defines the scaling applied to physical wave direction
     * values (in radians) prior to integer encoding. For example:
     * - `scaleFactorDirections = 2` implies a real scaling factor of
     *   \f$10^2\f$.
     */
    long scaleFactorDirections;

    /**
     * @brief Scaled integer representation of wave directions.
     *
     * Each element corresponds to a wave direction encoded as an integer,
     * obtained by scaling the physical direction value (in radians) by
     * \f$10^{\text{scaleFactorDirections}}\f$ and rounding to the nearest
     * integer.
     */
    std::vector<long> scaledValuesDirections;
};


namespace wave_direction_detail {

/**
 * @brief Compute a uniformly discretized wave direction grid.
 *
 * This function computes the angular discretization of wave propagation
 * directions over the interval \f$[0, 2\pi)\f$ using a midpoint rule.
 * The full circle is divided into `numberOfWaveDirections` equal angular
 * sectors, and each returned direction corresponds to the center of a sector.
 *
 * The discretization follows:
 * \f[
 *   \Delta\theta = \frac{2\pi}{N}, \qquad
 *   \theta_k = k\,\Delta\theta + \tfrac{1}{2}\Delta\theta,
 *   \quad k = 0,\dots,N-1
 * \f]
 *
 * This implementation is a direct C++ translation of the directional
 * discretization logic used in the ECMWF ECWAM wave model, originally
 * implemented in Fortran in:
 *   - `ecwam/src/ecwam/mfredir.F90`
 *
 * (see section "2. COMPUTATION OF DIRECTIONS, BANDWIDTH, SIN AND COS",
 * around line ~100 in the original source),
 * from the ECMWF ECWAM repository:
 *   - https://github.com/ecmwf-ifs/ecwam.git
 *
 * The numerical behavior of the original Fortran implementation is preserved
 * exactly, with the exception that only the directional angles are returned.
 * The computation of sine and cosine terms is intentionally omitted, as these
 * can be derived trivially by the caller if needed.
 *
 * @param[in] numberOfWaveDirections
 *   Total number of wave propagation directions used to discretize the
 *   directional space.
 *
 * @return
 *   Vector of size `numberOfWaveDirections` containing the wave directions
 *   in radians, uniformly distributed over \f$(0, 2\pi)\f$.
 *
 * @note
 *   The returned angles correspond to the centers of the angular bins
 *   (midpoint discretization), which is standard in spectral wave models.
 *
 * @note
 *   This function assumes a full directional coverage of \f$2\pi\f$ and
 *   does not support partial angular sectors.
 */
std::vector<double> compute_WaveDirectionGrid(long numberOfWaveDirections) {

    std::vector<double> th;
    th.resize(numberOfWaveDirections);

    const double delth = 2.0 * M_PI / static_cast<double>(numberOfWaveDirections);

    for (long k = 0; k < numberOfWaveDirections; ++k) {
        th[k] = static_cast<double>(k) * delth + 0.5 * delth;
    }

    return th;
}

/**
 * @brief Construct a scaled wave direction grid from physical direction values.
 *
 * This function builds a `WaveDirectionGrid` structure from a vector of
 * physical wave propagation directions expressed in radians. The physical
 * direction values are converted to an integer representation using a
 * base-10 logarithmic scaling factor.
 *
 * The scaling convention is defined as follows:
 * - `scaleFactorOfWaveDirections` represents the base-10 logarithm of the
 *   real scaling factor applied to the physical direction values.
 * - A value of `scaleFactorOfWaveDirections = 6` corresponds to a real
 *   scaling factor of \f$10^6\f$.
 *
 * Each scaled integer value is computed as:
 * \f[
 *   \text{scaledValue}_i =
 *     \mathrm{round}\!\left( \theta_i \times 10^{\text{scaleFactorOfWaveDirections}} \right)
 * \f]
 * where \f$\theta_i\f$ is the physical wave direction in radians.
 *
 * The resulting structure stores:
 * - the total number of wave directions,
 * - the logarithmic scaling factor,
 * - the vector of scaled integer direction values.
 *
 * This representation is typically used for serialization or encoding
 * purposes (e.g. GRIB), where integer storage with a known scaling factor
 * is required.
 *
 * @param[in] waveDirectionsInRadians
 *   Vector of physical wave propagation directions expressed in radians.
 *
 * @param[in] scaleFactorOfWaveDirections
 *   Base-10 logarithm of the scaling factor applied to the direction values
 *   prior to integer encoding.
 *
 * @return
 *   A `WaveDirectionGrid` structure containing the scaled integer
 *   representation of the input direction grid.
 *
 * @note
 *   No validation is performed on the input direction values (e.g. range
 *   checks within \f$[0, 2\pi)\f$) or on the scaling factor. The caller is
 *   responsible for ensuring that the provided values are physically
 *   meaningful and that the scaled values fit within the range of the
 *   target integer type.
 */
WaveDirectionGrid compute_WaveScaledDirectionGrid(const std::vector<double>& waveDirectionsInRadians,
                                                  long scaleFactorOfWaveDirections) {

    WaveDirectionGrid out{};

    out.numDirections         = static_cast<long>(waveDirectionsInRadians.size());
    out.scaleFactorDirections = scaleFactorOfWaveDirections;
    out.scaledValuesDirections.resize(waveDirectionsInRadians.size());
    for (long i = 0; i < static_cast<long>(waveDirectionsInRadians.size()); ++i) {
        out.scaledValuesDirections[i] =
            static_cast<long>(std::round(waveDirectionsInRadians[i] * std::pow(10.0, scaleFactorOfWaveDirections)));
    }

    return out;
}

}  // namespace wave_direction_detail

template <class OptDict_t, class MarsDict_t, class ParDict_t>
WaveDirectionGrid resolve_WaveDirectionGrid_or_throw(const OptDict_t& opt, const MarsDict_t& mars,
                                                     const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        WaveDirectionGrid out{};
        std::vector<double> waveDirectionsInRadians;

        // Must always be present (or defaulted to 2)
        long scaleFactorOfWaveDirections = get_opt<long>(par, "scaleFactorOfWaveDirections").value_or(2L);

        // Wave directions can be passed as full array in par dictionary
        bool hasWaveDirections = has(par, "waveDirections");

        // Or the information to reconstruct it must be present in par dictionary
        bool hasNumberOfWaveDirections = has(par, "numberOfWaveDirections");

        if (hasWaveDirections) {
            waveDirectionsInRadians = get_or_throw<std::vector<double>>(par, "waveDirections");

            // Logging of the waveDirectionGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "waveDirectionGrid: look up from Par dictionary as vector";
                return logMsg;
            }());
        }
        else if (hasNumberOfWaveDirections) {
            long numberOfWaveDirections = get_or_throw<long>(par, "numberOfWaveDirections");

            waveDirectionsInRadians =
                wave_direction_detail::compute_WaveDirectionGrid(static_cast<std::size_t>(numberOfWaveDirections));

            // Logging of the waveDirectionGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "waveDirectionGrid: reconstruct from Par dictionary with numberOfWaveDirections=";
                logMsg += std::to_string(numberOfWaveDirections);
                return logMsg;
            }());
        }
        else {
            throw Mars2GribDeductionException(
                "Insufficient wave direction information in Par dictionary: "
                "neither waveDirections nor numberOfWaveDirections is present",
                Here());
        }

        // Build the scaled direction grid
        out = wave_direction_detail::compute_WaveScaledDirectionGrid(waveDirectionsInRadians,
                                                                     scaleFactorOfWaveDirections);

        return out;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get wave direction information from Mars and Par dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
}


}  // namespace metkit::mars2grib::backend::deductions
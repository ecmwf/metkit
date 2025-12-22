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
 * @brief Metadata and scaled representation of a wave frequency discretization.
 *
 * This structure describes a wave frequency grid together with its integer,
 * scaled representation. It is intended for use in contexts where frequencies
 * must be stored or transmitted as integers (e.g. GRIB encoding), while
 * retaining a well-defined mapping to physical frequency values in Hz.
 *
 * The scaling convention used is logarithmic:
 * - `scaleFactorFrequencies` represents the base-10 logarithm of the real
 *   scaling factor applied to the physical frequencies.
 * - A value of `scaleFactorFrequencies = 6` corresponds to a real scaling
 *   factor of \f$10^6\f$.
 *
 * Scaled integer values are obtained by:
 * \f[
 *   \text{scaledValue}_i =
 *     \mathrm{round}\!\left( f_i \times 10^{\text{scaleFactorFrequencies}} \right)
 * \f]
 * where \f$f_i\f$ is the physical frequency in Hz.
 *
 * The inverse operation (reconstruction of physical frequencies) is:
 * \f[
 *   f_i =
 *     \frac{\text{scaledValue}_i}{10^{\text{scaleFactorFrequencies}}}
 * \f]
 *
 * @note
 *   This structure is a plain data container and does not enforce consistency
 *   between `numFrequencies` and the size of `scaledValuesFrequencies`.
 *   Such validation is expected to be performed by the caller.
 */
struct WaveFrequencyGrid {

    /**
     * @brief Number of discrete wave frequencies.
     */
    long numFrequencies;

    /**
     * @brief Base-10 logarithm of the real frequency scaling factor.
     *
     * This value defines the scaling applied to physical frequencies prior to
     * integer encoding. For example:
     * - `scaleFactorFrequencies = 6` implies a real scaling factor of \f$10^6\f$.
     */
    long scaleFactorFrequencies;

    /**
     * @brief Scaled integer representation of wave frequencies.
     *
     * Each element is the rounded value of the corresponding physical frequency
     * (in Hz) multiplied by \f$10^{\text{scaleFactorFrequencies}}\f$.
     */
    std::vector<long> scaledValuesFrequencies;
};


namespace wave_frequency_detail {


/**
 * @brief Compute a geometrically spaced wave frequency grid.
 *
 * This function constructs a frequency grid using geometric spacing,
 * centered around a reference wave frequency. The reference frequency
 * is placed at a specified index, and lower and higher frequencies are
 * obtained by successive division or multiplication by a constant ratio.
 *
 * The discretization follows:
 * - fr[indexOfReferenceWaveFrequency] = referenceWaveFrequency
 * - fr[i-1] = fr[i] / waveFrequencySpacingRatio
 * - fr[i+1] = fr[i] * waveFrequencySpacingRatio
 *
 * This function is a direct C++ translation of the original Fortran routine
 * `MFR` located in:
 *   - `ecwam/src/ecwam/mfr.F90`
 *
 * from the ECMWF ECWAM model repository:
 *   - https://github.com/ecmwf-ifs/ecwam.git
 *
 * The numerical behavior and discretization logic are preserved exactly.
 *
 * @param[in] numberOfWaveFrequencies
 *   Total number of wave frequencies to generate.
 *
 * @param[in] indexOfReferenceWaveFrequency
 *   Index (1-based) of the reference wave frequency.
 *
 * @param[in] referenceWaveFrequency
 *   Reference wave frequency value.
 *
 * @param[in] waveFrequencySpacingRatio
 *   Geometric ratio between two consecutive wave frequencies.
 *
 * @return
 *   Vector containing the discretized wave frequency grid.
 *
 * @throws std::out_of_range
 *   If `indexOfReferenceWaveFrequency` is outside the valid range.
 *
 * @note
 *   The reference index follows Fortran conventions (1-based) to preserve
 *   compatibility with legacy configurations.
 *
 * @note
 *   This implementation assumes geometric spacing of frequencies and does
 *   not perform validation of the physical consistency of the input values.
 */
std::vector<double> compute_WaveFrequencyGrid(long numberOfWaveFrequencies, long indexOfReferenceWaveFrequency,
                                              double referenceWaveFrequency, double waveFrequencySpacingRatio) {

    if (indexOfReferenceWaveFrequency == 0 || indexOfReferenceWaveFrequency > numberOfWaveFrequencies) {
        throw std::out_of_range("indexOfReferenceWaveFrequency out of range");
    }

    std::vector<double> waveFrequencies(numberOfWaveFrequencies);

    // Convert reference index from 1-based (Fortran-style) to 0-based
    const long ref = indexOfReferenceWaveFrequency - 1;

    // Reference frequency
    waveFrequencies[ref] = referenceWaveFrequency;

    // Frequencies below the reference
    for (long i = ref; i-- > 0;) {
        waveFrequencies[i] = waveFrequencies[i + 1] / waveFrequencySpacingRatio;
    }

    // Frequencies above the reference
    for (long i = ref + 1; i < numberOfWaveFrequencies; ++i) {
        waveFrequencies[i] = waveFrequencySpacingRatio * waveFrequencies[i - 1];
    }

    return waveFrequencies;
}

/**
 * @brief Construct a scaled wave frequency grid from physical frequency values.
 *
 * This function builds a `WaveFrequencyGrid` structure from a vector of
 * physical wave frequencies expressed in Hertz. The physical values are
 * converted to an integer representation using a base-10 logarithmic
 * scaling factor.
 *
 * The scaling convention is defined as follows:
 * - `scaleFactorOfWaveFrequencies` represents the base-10 logarithm of the
 *   real scaling factor applied to the physical frequencies.
 * - A value of `scaleFactorOfWaveFrequencies = 6` corresponds to a real
 *   scaling factor of \f$10^6\f$.
 *
 * Each scaled integer value is computed as:
 * \f[
 *   \text{scaledValue}_i =
 *     \mathrm{round}\!\left( f_i \times 10^{\text{scaleFactorOfWaveFrequencies}} \right)
 * \f]
 * where \f$f_i\f$ is the physical frequency in Hz.
 *
 * The resulting structure stores:
 * - the total number of frequencies,
 * - the scaling factor (logarithmic form),
 * - the vector of scaled integer frequency values.
 *
 * This representation is typically used for serialization or encoding
 * purposes (e.g. GRIB), where integer storage with a known scaling factor
 * is required.
 *
 * @param[in] waveFrequenciesInHz
 *   Vector of physical wave frequencies expressed in Hertz.
 *
 * @param[in] scaleFactorOfWaveFrequencies
 *   Base-10 logarithm of the scaling factor applied to the frequencies
 *   prior to integer encoding.
 *
 * @return
 *   A `WaveFrequencyGrid` structure containing the scaled integer
 *   representation of the input frequency grid.
 *
 * @note
 *   No validation is performed on the input frequencies (e.g. positivity,
 *   monotonicity) or on the scaling factor. The caller is responsible for
 *   ensuring that the provided values are physically meaningful and that
 *   the scaled values fit within the range of the target integer type.
 */
WaveFrequencyGrid compute_WaveScaledFrequencyGrid(const std::vector<double>& waveFrequenciesInHz,
                                                  long scaleFactorOfWaveFrequencies) {

    WaveFrequencyGrid out{};

    out.numFrequencies         = static_cast<long>(waveFrequenciesInHz.size());
    out.scaleFactorFrequencies = scaleFactorOfWaveFrequencies;
    out.scaledValuesFrequencies.resize(waveFrequenciesInHz.size());
    for (long i = 0; i < static_cast<long>(waveFrequenciesInHz.size()); ++i) {
        out.scaledValuesFrequencies[i] =
            static_cast<long>(std::round(waveFrequenciesInHz[i] * std::pow(10.0, scaleFactorOfWaveFrequencies)));
    }

    return out;
}


}  // namespace wave_frequency_detail


/**
 * @brief Resolve and construct the wave frequency grid with scaled representation.
 *
 * This deduction resolves the wave frequency grid required for wave spectral
 * encoding, producing a `WaveFrequencyGrid` structure containing both the
 * discretization metadata and the scaled integer representation of frequencies.
 *
 * The wave frequency grid can be obtained in two alternative ways from the
 * parameter dictionary (`par`):
 *
 * 1. **Direct specification**
 *    If the key `waveFrequencies` is present, it is interpreted as the full
 *    frequency grid expressed in Hertz and used directly.
 *
 * 2. **Reconstruction from spacing parameters**
 *    If `waveFrequencies` is not present, the grid is reconstructed provided
 *    that all of the following keys are available:
 *    - `numberOfWaveFrequencies`
 *    - `indexOfReferenceWaveFrequency`
 *    - `referenceWaveFrequency`
 *    - `waveFrequencySpacingRatio`
 *
 *    In this case, the frequency grid is reconstructed using a geometric
 *    spacing centered around the reference frequency, following the same
 *    discretization logic as the legacy Fortran implementation.
 *
 * In both cases, the resulting frequency grid (in Hz) is converted to a
 * scaled integer representation using a base-10 logarithmic scaling factor.
 * The scaling factor is obtained from:
 *   - `scaleFactorOfWaveFrequencies` (optional, defaults to 6 if not provided).
 *
 * The scaled representation is built according to:
 * \f[
 *   \text{scaledValue}_i =
 *     \mathrm{round}\!\left( f_i \times 10^{\text{scaleFactorOfWaveFrequencies}} \right)
 * \f]
 *
 * All errors encountered during lookup, reconstruction, or scaling are
 * propagated using nested exceptions to preserve full diagnostic context.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (currently unused).
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (currently unused).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary providing wave frequency information.
 *
 * @param[in] opt
 *   Options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary (unused by this deduction).
 *
 * @param[in] par
 *   Parameter dictionary containing wave frequency information, either as
 *   a full array or as reconstruction parameters.
 *
 * @return
 *   A fully populated `WaveFrequencyGrid` structure containing:
 *   - the number of wave frequencies,
 *   - the logarithmic scaling factor,
 *   - the vector of scaled integer frequency values.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - neither `waveFrequencies` nor the full set of reconstruction parameters
 *     is present in the parameter dictionary,
 *   - any required key is missing or has an invalid type,
 *   - reconstruction or scaling of the frequency grid fails,
 *   - any unexpected error occurs during deduction.
 *
 * @note
 *   If both `waveFrequencies` and reconstruction parameters are present,
 *   the explicit `waveFrequencies` vector takes precedence.
 *
 * @note
 *   This deduction does not validate the physical correctness of the
 *   frequency grid (e.g. positivity, monotonicity). Such validation is
 *   expected to be handled at a higher level.
 *
 * @note
 *   The implementation follows a fail-fast strategy and uses nested
 *   exceptions to preserve detailed error context across API boundaries.
 */
template <class OptDict_t, class MarsDict_t, class ParDict_t>
WaveFrequencyGrid resolve_WaveFrequencyGrid_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                     const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        WaveFrequencyGrid out{};
        std::vector<double> waveFrequenciesInHz;

        // Must always be present (or defaulted to 6)
        long scaleFactorOfWaveFrequencies = get_opt<long>(par, "scaleFactorOfWaveFrequencies").value_or(6L);

        // Wave frequencies can be passed as full array in par dictionary
        bool hasWaveFrequencies = has(par, "waveFrequencies");

        // Or the information to reconstruct it must be present in par dictionary
        bool hasNumberOfWaveFrequencies       = has(par, "numberOfWaveFrequencies");
        bool hasIndexOfReferenceWaveFrequency = has(par, "indexOfReferenceWaveFrequency");
        bool hasReferenceWaveFrequency        = has(par, "referenceWaveFrequency");
        bool hasWaveFrequencySpacingRatio     = has(par, "waveFrequencySpacingRatio");

        bool canReconstructWaveFrequencies = hasNumberOfWaveFrequencies && hasIndexOfReferenceWaveFrequency &&
                                             hasReferenceWaveFrequency && hasWaveFrequencySpacingRatio;
        if (hasWaveFrequencies) {
            // Get the wave frequency grid in Hz directly
            waveFrequenciesInHz = get_or_throw<std::vector<double>>(par, "waveFrequencies");

            // Logging of the waveFrequencyGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "waveFrequencyGrid: look up from Par dictionary as vector";
                return logMsg;
            }());
        }
        else if (canReconstructWaveFrequencies && !hasWaveFrequencies) {
            // Reconstruct the wave frequency grid in Hz
            long numberOfWaveFrequencies       = get_or_throw<long>(par, "numberOfWaveFrequencies");
            long indexOfReferenceWaveFrequency = get_or_throw<long>(par, "indexOfReferenceWaveFrequency");
            double referenceWaveFrequency      = get_or_throw<double>(par, "referenceWaveFrequency");
            double waveFrequencySpacingRatio   = get_or_throw<double>(par, "waveFrequencySpacingRatio");

            waveFrequenciesInHz =
                wave_frequency_detail::compute_WaveFrequencyGrid(numberOfWaveFrequencies, indexOfReferenceWaveFrequency,
                                                                 referenceWaveFrequency, waveFrequencySpacingRatio);

            // Logging of the waveFrequencyGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "waveFrequencyGrid: reconstruct from Par dictionary with params={";
                logMsg += std::to_string(numberOfWaveFrequencies) + ", ";
                logMsg += std::to_string(indexOfReferenceWaveFrequency) + ", ";
                logMsg += std::to_string(referenceWaveFrequency) + ", ";
                logMsg += std::to_string(waveFrequencySpacingRatio) + "}";
                return logMsg;
            }());
        }
        else {
            throw Mars2GribDeductionException(
                "Insufficient Mars/Par dictionaries: neither numberOfWaveFrequencies nor waveFrequencies are present",
                Here());
        }

        // Build the scaled frequency grid
        out = wave_frequency_detail::compute_WaveScaledFrequencyGrid(waveFrequenciesInHz, scaleFactorOfWaveFrequencies);

        return out;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get wave frequency information from Mars and Par dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions
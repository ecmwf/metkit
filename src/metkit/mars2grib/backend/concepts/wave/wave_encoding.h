#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/wave/wave_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsDirection.h"
#include "metkit/mars2grib/backend/deductions/marsFrequency.h"
#include "metkit/mars2grib/backend/deductions/periodItMax.h"
#include "metkit/mars2grib/backend/deductions/periodItMin.h"
#include "metkit/mars2grib/backend/deductions/waveSpectraInfo.h"
#include "metkit/mars2grib/utils/waveUtils.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, WaveType Variant>
constexpr bool waveApplicable() {
    bool condition1 =
        (Section == SecProductDefinitionSection && Stage == StageAllocate && Variant == WaveType::Spectra);

    bool condition2 = (Section == SecProductDefinitionSection && Stage == StagePreset && Variant == WaveType::Period);

    bool condition3 = (Section == SecProductDefinitionSection && Stage == StageRuntime && Variant == WaveType::Spectra);

    return condition1 || condition2 || condition3;
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, WaveType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void WaveOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;
    using metkit::mars2grib::utils::wave::WaveSpectraInfo;

    if constexpr (waveApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Wave] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(waveTypeName<Variant>()) << std::endl;

            // Checks
            if constexpr (Variant == WaveType::Spectra) {
                checks::matchProductDefinitionTemplateNumber_or_throw(opt, out, {99, 100});
            }
            else if constexpr (Variant == WaveType::Period) {
                checks::matchProductDefinitionTemplateNumber_or_throw(opt, out, {103, 104});
            }

            // =============================================================
            if constexpr (Stage == StageAllocate) {

                if constexpr (Variant == WaveType::Spectra) {

                    // Get directional and frequency grid information
                    WaveSpectraInfo spectraInfo = deductions::waveSpectraInfo_or_throw(opt, mars, par);

                    // Set directions informations
                    set_or_throw<long>(out, "numberOfWaveDirections", spectraInfo.numDirections);
                    set_or_throw<long>(out, "scaleFactorOfWaveDirections", spectraInfo.scaleFactorDirections);
                    set_or_throw<std::vector<long>>(out, "scaledValuesOfWaveDirections",
                                                    spectraInfo.scaledValuesDirections);

                    // Set frequencies information
                    set_or_throw<long>(out, "numberOfWaveFrequencies", spectraInfo.numFrequencies);
                    set_or_throw<long>(out, "scaleFactorOfWaveFrequencies", spectraInfo.scaleFactorFrequencies);
                    set_or_throw<std::vector<long>>(out, "scaledValuesOfWaveFrequencies",
                                                    spectraInfo.scaledValuesFrequencies);

                }  // if constexpr ( Variant == WaveType::Period )
            }

            if constexpr (Stage == StagePreset) {

                if constexpr (Variant == WaveType::Period) {

                    // Get wave period information
                    std::optional<long> itMin = deductions::periodItMin_opt(mars, par);
                    std::optional<long> itMax = deductions::periodItMin_opt(mars, par);

                    // Set wave period information
                    // This information is set by eccodes as part of the paramId, not really
                    // sure it make sense to (over)write it here...
                    if (itMin.has_value() && itMax.has_value()) {
                        set_or_throw<long>(out, "typeOfWavePeriodInterval", 7L);
                        set_or_throw<long>(out, "scaleFactorOfLowerWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfLowerWavePeriodLimit", itMin.value());
                        set_or_throw<long>(out, "scaleFactorOfUpperWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfUpperWavePeriodLimit", itMax.value());
                    }
                    else if (itMin.has_value() && !itMax.has_value()) {
                        set_or_throw<long>(out, "typeOfWavePeriodInterval", 3L);
                        set_or_throw<long>(out, "scaleFactorOfLowerWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfLowerWavePeriodLimit", itMin.value());
                    }
                    else if (!itMin.has_value() && itMax.has_value()) {
                        set_or_throw<long>(out, "typeOfWavePeriodInterval", 4L);
                        set_or_throw<long>(out, "scaleFactorOfUpperWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfUpperWavePeriodLimit", itMax.value());
                    }

                }  // if constexpr ( Variant == WaveType::Period )
            }

            if constexpr (Stage == StageRuntime) {

                // Nothing to do at runtime for now
                if constexpr (Variant == WaveType::Spectra) {
                    // Nothing to do at runtime for now
                    long marsDir  = deductions::marsDirection_or_throw(mars, par);
                    long marsFreq = deductions::marsFrequency_or_throw(mars, par);

                    set_or_throw<long>(out, "numberOfWaveDirections", marsDir);
                    set_or_throw<long>(out, "numberOfWaveFrequencies", marsFreq);
                }
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(waveName), std::string(waveTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `ensemble` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( waveApplicable<Stage, Section, Variant>() )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(waveName), std::string(waveTypeName<Variant>()), std::to_string(Stage),
                                    std::to_string(Section), "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

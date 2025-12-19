#pragma once

#include <iostream>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "satellite_enum.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsChannel.h"
#include "metkit/mars2grib/backend/deductions/marsIdent.h"
#include "metkit/mars2grib/backend/deductions/marsInstrument.h"
#include "metkit/mars2grib/backend/deductions/satelliteSeries.h"
#include "metkit/mars2grib/backend/deductions/scaleFactorOfCentralWaveNumber.h"
#include "metkit/mars2grib/backend/deductions/scaledValueOfCentralWaveNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, SatelliteType Variant>
constexpr bool satelliteApplicable() {

    bool condition1 = (Stage == StagePreset && Section == SecLocalUseSection);
    bool condition2 = (Stage == StageAllocate && Section == SecProductDefinitionSection);
    bool condition3 = (Stage == StagePreset && Section == SecProductDefinitionSection);


    return (condition1 || condition2 || condition3);
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, SatelliteType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void SatelliteOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                 OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (satelliteApplicable<Stage, Section, Variant>()) {

        try {

            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Satellite] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(satelliteTypeName<Variant>()) << std::endl;

            if constexpr (Section == SecLocalUseSection && Stage == StagePreset) {

                // Verify section 2
                checks::matchLocalDefinitionNumber_or_throw(opt, out, {14});

                long channel = deductions::marsChannel_or_throw(mars, par);
                set_or_throw(out, "channel", channel);
            }

            if constexpr (Section == SecProductDefinitionSection && Stage == StageAllocate) {

                // Verify section 4
                checks::matchProductDefinitionTemplateNumber_or_throw(opt, out, {32, 33});

                // Allocate number of signals
                set_or_throw<long>(out, "numberOfContributingSpectralBands", 1L);
            }

            if constexpr (Section == SecProductDefinitionSection && Stage == StagePreset) {

                // Verify section 4
                checks::matchProductDefinitionTemplateNumber_or_throw(opt, out, {32, 33});

                long satelliteNumber = deductions::marsIdent_or_throw(mars, par);
                long instrumentType  = deductions::marsInstrument_or_throw(mars, par);

                long satelliteSeries                = deductions::satelliteSeries_or_throw(mars, par);
                long scaleFactorOfCentralWaveNumber = deductions::scaleFactorOfCentralWaveNumber_or_throw(mars, par);
                long scaledValueOfCentralWaveNumber = deductions::scaledValueOfCentralWaveNumber_or_throw(mars, par);

                set_or_throw<long>(out, "satelliteSeries", satelliteSeries);
                set_or_throw<long>(out, "satelliteNumber", satelliteNumber);
                set_or_throw<long>(out, "instrumentType", instrumentType);
                set_or_throw<long>(out, "scaleFactorOfCentralWaveNumber", scaleFactorOfCentralWaveNumber);
                set_or_throw<long>(out, "scaledValueOfCentralWaveNumber", scaledValueOfCentralWaveNumber);
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(satelliteName), std::string(satelliteTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `satellite` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(satelliteName), std::string(satelliteTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

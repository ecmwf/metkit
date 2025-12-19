#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/generating-process/generating_process_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/backgroundProcess.h"
#include "metkit/mars2grib/backend/deductions/generatingProcessIdentifier.h"
#include "metkit/mars2grib/backend/deductions/typeOfGeneratingProcess.h"


// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {
// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, GeneratingProcessType Variant>
constexpr bool generating_processApplicable() {
    return (Stage == StagePreset || Variant == GeneratingProcessType::Default);
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, GeneratingProcessType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void GeneratingProcessOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                         OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (generating_processApplicable<Stage, Section, Variant>()) {

        try {

            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept GeneratingProcess] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(generating_processTypeName<Variant>()) << std::endl;

            // Retrieve the information
            std::optional<long> generatingProcessIdentifier = deductions::generatingProcessIdentifier_opt(mars, par);
            std::optional<long> typeOfGeneratingProcess     = deductions::typeOfGeneratingProcess_opt(mars, par);
            std::optional<long> backgroundProcess           = deductions::backgroundProcess_opt(mars, par);

            // TODO MIVAL: To be verified
            if (bool useModel = get_opt<bool>(opt, "useModelnGeneratingProcess").value_or(true); useModel) {
                std::optional<long> modelOpt = get_opt<long>(mars, "model");
                if (modelOpt.has_value()) {
                    set_or_throw<long>(out, "generatingPProcessIdentifier", modelOpt.value());
                }
            }

            // Set the values only if defined
            if (generatingProcessIdentifier.has_value()) {
                set_or_throw<long>(out, "generatingProcessIdentifier", generatingProcessIdentifier.value());
            }

            if (typeOfGeneratingProcess.has_value()) {
                set_or_throw<long>(out, "typeOfGeneratingProcess", typeOfGeneratingProcess.value());
            }

            if (backgroundProcess.has_value()) {
                set_or_throw<long>(out, "backgroundProcess", backgroundProcess.value());
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(std::string(generatingProcessName),
                                                             std::string(generating_processTypeName<Variant>()),
                                                             std::to_string(Stage), std::to_string(Section),
                                                             "Unable to set `generating_process` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(generatingProcessName),
                                    std::string(generating_processTypeName<Variant>()), std::to_string(Stage),
                                    std::to_string(Section), "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

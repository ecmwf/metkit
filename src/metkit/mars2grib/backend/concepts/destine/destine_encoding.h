#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/destine/destine_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsActivity.h"
#include "metkit/mars2grib/backend/deductions/marsExperiment.h"
#include "metkit/mars2grib/backend/deductions/marsGeneration.h"
#include "metkit/mars2grib/backend/deductions/marsModel.h"
#include "metkit/mars2grib/backend/deductions/marsRealization.h"
#include "metkit/mars2grib/backend/deductions/marsResolution.h"

// checks
#include "metkit/mars2grib/backend/checks/matchDataset.h"
#include "metkit/mars2grib/backend/checks/matchDestineLocalSection.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, DestineType Variant>
constexpr bool destineApplicable() {
    // Confitions to apply concept
    return ((Variant == DestineType::ClimateDT || Variant == DestineType::ExtremesDT) && (Stage == StagePreset) &&
            (Section == SecLocalUseSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, DestineType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void DestineOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
               OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (destineApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Destine] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(destineTypeName<Variant>()) << std::endl;

            // =============================================================
            // Preconditions/contracts for this concept
            checks::matchDestineLocalSection_or_throw(opt, out);

            if constexpr (Variant == DestineType::ExtremesDT) {

                // check consistency of dataset
                checks::matchDataset_or_throw(mars, par, "extremes-dt");

                // Set values in output dictionary
                set_or_throw<std::string>(out, "dataset", "extremes-dt");
            }
            else if constexpr (Variant == DestineType::ClimateDT) {

                // check consistency of dataset
                checks::matchDataset_or_throw(mars, par, "climate-dt");

                // Set values in output dictionary
                set_or_throw<std::string>(out, "dataset", "climate-dt");

                // Read mars keywords from dictionary
                std::string activityVal   = deductions::marsActivity(mars, par);
                std::string experimentVal = deductions::marsExperiment(mars, par);
                std::string resolutionVal = deductions::marsResolution(mars, par);
                std::string modelVal      = deductions::marsModel(mars, par);

                long generationVal  = deductions::marsGeneration(mars, par);
                long realizationVal = deductions::marsRealization(mars, par);

                // Set values in output dictionary
                set_or_throw<std::string>(out, "activity", activityVal);
                set_or_throw<std::string>(out, "experiment", experimentVal);
                set_or_throw<std::string>(out, "resolution", resolutionVal);
                set_or_throw<std::string>(out, "model", modelVal);
                set_or_throw<long>(out, "generation", generationVal);
                set_or_throw<long>(out, "realization", realizationVal);
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(destineName), std::string(destineTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `analysis` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(destineName), std::string(destineTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

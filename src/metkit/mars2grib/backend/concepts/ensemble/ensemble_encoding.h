#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/ensemble/ensemble_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsNumber.h"
#include "metkit/mars2grib/backend/deductions/numberOfForecastsInEnsemble.h"
#include "metkit/mars2grib/backend/deductions/typeOfEnsembleForecast.h"

// checks
#include "metkit/mars2grib/backend/checks/isEnsembleProductDefinitionTemplateNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, EnsembleType Variant>
constexpr bool ensembleApplicable() {
    // Confitions to apply concept
    return ((Variant == EnsembleType::Individual) && (Stage == StagePreset) &&
            (Section == SecProductDefinitionSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, EnsembleType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void EnsembleOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (ensembleApplicable<Stage, Section, Variant>()) {

        try {

            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Ensemble] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(ensembleTypeName<Variant>()) << std::endl;

            if constexpr (Variant == EnsembleType::Individual) {

                // =============================================================
                // Checks
                checks::isEnsembleProductDefinitionTemplateNumber_or_throw(opt, out);

                // =============================================================
                // Deduce typeOfEnsembleForecast from mars dictionary
                deductions::TypeOfEnsembleForecast typeOfEnsembleForecast =
                    deductions::typeOfEnsembleForecast(mars, par);
                long numberOfForecastsInEnsemble = deductions::numberOfForecastsInEnsemble(mars, par);
                long marsNumber                  = deductions::marsNumber(mars, par);


                // Set grib key
                set_or_throw<long>(out, "typeOfEnsembleForecast", static_cast<long>(typeOfEnsembleForecast));
                set_or_throw<long>(out, "perturbationNumber", marsNumber);
                set_or_throw<long>(out, "numberOfForecastsInEnsemble", numberOfForecastsInEnsemble);
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(ensembleName), std::string(ensembleTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `ensemble` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(ensembleName), std::string(ensembleTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

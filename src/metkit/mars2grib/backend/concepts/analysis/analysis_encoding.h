#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/analysis/analysis_enum.h"
#include "metkit/mars2grib/backend/concepts/concept_core.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/lengthOfTimeWindow.h"
#include "metkit/mars2grib/backend/deductions/marsAnoffset.h"

// checks
#include "metkit/mars2grib/backend/checks/matchLocalDefinitionNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, AnalysisType Variant>
constexpr bool analysisApplicable() {

    // Conditions to apply concept
    return ((Variant == AnalysisType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, AnalysisType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void AnalysisOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (analysisApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Analysis] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(analysisTypeName<Variant>()) << std::endl;

            // =============================================================
            // Preconditions/contracts for this concept
            checks::matchLocalDefinitionNumber_or_throw(opt, out, {36L});

            // =============================================================
            // Get values from input MARS dictionary
            auto anoffsetVal           = deductions::marsAnoffset(mars, par);
            auto lengthOfTimeWindowOpt = deductions::lengthOfTimeWindow_opt(mars, par);

            // =============================================================
            // Set values in output GRIB dictionary
            set_or_throw<long>(out, "offsetToEndOf4DvarWindow", anoffsetVal);

            // Optional arguments
            if (lengthOfTimeWindowOpt.has_value()) {
                set_or_throw<long>(out, "lengthOf4DvarWindow", lengthOfTimeWindowOpt.value());
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(analysisName), std::string(analysisTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `analysis` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(analysisName), std::string(analysisTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

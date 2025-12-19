#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/composition/composition_enum.h"
#include "metkit/mars2grib/backend/concepts/concept_core.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsChem.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, CompositionType Variant>
constexpr bool compositionApplicable() {
    return (Stage == StagePreset && Section == SecProductDefinitionSection);
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, CompositionType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void CompositionOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                   OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (compositionApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Composition] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(compositionTypeName<Variant>()) << std::endl;

            if constexpr (Variant == CompositionType::Chem) {

                // Get the `chem` from Mars dictionary
                long chemVal = deductions::marsChem_or_throw<MarsDict_t, ParDict_t>(mars, par);

                // Set the composition field
                set_or_throw<long>(out, "constituentType", chemVal);
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(compositionName), std::string(compositionTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `composition` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(compositionName), std::string(compositionTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

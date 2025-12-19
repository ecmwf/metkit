#pragma once

#include <iostream>
#include <string>
#include <string_view>


// Logging
#include "metkit/config/LibMetkit.h"

// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrange_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsMethod.h"
#include "metkit/mars2grib/backend/deductions/marsSystem.h"

// checks
#include "metkit/mars2grib/backend/checks/matchLocalDefinitionNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, LongrangeType Variant>
constexpr bool longrangeApplicable() {
    return ((Variant == LongrangeType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, LongrangeType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void LongrangeOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                 OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (longrangeApplicable<Stage, Section, Variant>()) {


        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Longrange] Applying longrange encoding"
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(longrangeTypeName<Variant>()) << std::endl;

            // =============================================================
            // Preconditions/contracts for this concept
            checks::matchLocalDefinitionNumber_or_throw(opt, out, {15L});

            // =============================================================
            // Get values from input MARS dictionary
            auto methodVal = deductions::marsMethod(mars, par);
            auto systemVal = deductions::marsSystem(mars, par);


            // =============================================================
            // Set values in output GRIB dictionary
            set_or_throw<long>(out, "methodNumber", methodVal);
            set_or_throw<long>(out, "systemNumber", systemVal);
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(longrangeName), std::string(longrangeTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `longrange` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( marsApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(longrangeName), std::string(longrangeTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

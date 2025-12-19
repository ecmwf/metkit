#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/mars/mars_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsClass.h"
#include "metkit/mars2grib/backend/deductions/marsExpver.h"
#include "metkit/mars2grib/backend/deductions/marsStream.h"
#include "metkit/mars2grib/backend/deductions/marsType.h"

// checks
#include "metkit/mars2grib/backend/checks/hasLocalUseSection.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, MarsType Variant>
constexpr bool marsApplicable() {

    // Confitions to apply concept
    return ((Variant == MarsType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, MarsType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void MarsOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::check;
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    // eccodes/definitions/grib2/local.98.36.def
    if constexpr (marsApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Mars] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(marsTypeName<Variant>()) << std::endl;

            // =============================================================
            // Checks
            checks::hasLocalUseSection_or_throw(opt, out);

            // =============================================================
            // Get values from input MARS dictionary
            std::string marsClassVal  = deductions::marsClass(mars, par);
            std::string marsTypeVal   = deductions::marsType(mars, par);
            std::string marsStreamVal = deductions::marsStream(mars, par);
            std::string marsExpverVal = deductions::marsExpver(mars, par);

            // =============================================================
            // Set values in output dictionary
            set_or_throw<std::string>(out, "class", marsClassVal);
            set_or_throw<std::string>(out, "type", marsTypeVal);
            set_or_throw<std::string>(out, "stream", marsStreamVal);
            set_or_throw<std::string>(out, "expver", marsExpverVal);
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(marsName), std::string(marsTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `mars` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( marsApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(marsName), std::string(marsTypeName<Variant>()), std::to_string(Stage),
                                    std::to_string(Section), "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

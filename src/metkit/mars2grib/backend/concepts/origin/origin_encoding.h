#pragma once

#include <exception>
#include <iostream>
#include <string>


// Logging
#include "metkit/config/LibMetkit.h"

// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/origin/origin_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/centre.h"
#include "metkit/mars2grib/backend/deductions/subCentre.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, OriginType Variant>
constexpr bool originApplicable() {

    // Conditions to apply concept
    return ((Variant == OriginType::Default) && (Stage == StagePreset) && (Section == SecLocalUseSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, OriginType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void OriginOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
              OutDict_t& out) noexcept(false) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (originApplicable<Stage, Section, Variant>()) {

        try {

            // Debug output
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Origin] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(originTypeName<Variant>()) << std::endl;

            // Deduction rules
            std::string centre = deductions::centre(mars, par);
            long subCentre     = deductions::sub_centre(mars, par);

            // Set values in output dictionary (grib sample)
            set_or_throw<std::string>(out, "origin", centre);
            set_or_throw<long>(out, "subCentre", subCentre);
        }
        catch (...) {

            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(originName), std::string(originTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `origin` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( originApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(originName), std::string(originTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

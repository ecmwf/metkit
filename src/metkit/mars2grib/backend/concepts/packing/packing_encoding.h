#pragma once

#include <iostream>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/packing/packing_enum.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchDataRepresentationTemplateNumber.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/bitsPerValue.h"
#include "metkit/mars2grib/backend/deductions/laplacianOperator.h"
#include "metkit/mars2grib/backend/deductions/subSetTrunc.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
// ======================================================
template <std::size_t Stage, std::size_t Section, PackingType Variant>
constexpr bool packingApplicable() {
    return (Stage == StagePreset && Section == SecDataRepresentationSection);
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, PackingType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void PackingOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
               OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (packingApplicable<Stage, Section, Variant>()) {

        try {

            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Packing] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(packingTypeName<Variant>()) << std::endl;

            if constexpr (Variant == PackingType::Simple) {

                // Check sample structure
                checks::matchDataRepresentationTemplateNumber_or_throw(opt, out, {0});

                // Get bits per value
                long bitsPerValue = deductions::bitsPerValue_or_throw(mars, par);

                // Set bits per value
                set_or_throw<long>(out, "bitsPerValue", bitsPerValue);
            }

            if constexpr (Variant == PackingType::Ccsds) {

                // Check sample structure
                checks::matchDataRepresentationTemplateNumber_or_throw(opt, out, {42});

                // Get bits per value
                long bitsPerValue = deductions::bitsPerValue_or_throw(mars, par);

                // Set bits per value
                set_or_throw<long>(out, "bitsPerValue", bitsPerValue);
            }

            if constexpr (Variant == PackingType::SpectralComplex) {

                // Check sample structure
                checks::matchDataRepresentationTemplateNumber_or_throw(opt, out, {51});

                // Get bits per value
                long bitsPerValue            = deductions::bitsPerValue_or_throw(mars, par);
                const auto laplacianOperator = deductions::laplacianOperator_or_throw(mars, par);
                long trunc                   = deductions::subSetTrunc_or_throw(mars, par);


                // Set bits per value
                set_or_throw<long>(out, "bitsPerValue", bitsPerValue);
                set_or_throw(out, "laplacianOperator", laplacianOperator);
                set_or_throw<long>(out, "subSetJ", trunc);
                set_or_throw<long>(out, "subSetK", trunc);
                set_or_throw<long>(out, "subSetM", trunc);
                set_or_throw<long>(out, "TS", (trunc + 1) * (trunc + 2));
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(packingName), std::string(packingTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `packing` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( packingApplicable<Stage, Section, Variant>() )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(packingName), std::string(packingTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

#pragma once

#include <iostream>
#include <string>
#include <string_view>

#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/shape-of-the-earth/shape_of_the_earth_enum.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, ShapeOfTheEarthType Variant>
constexpr bool shape_of_the_earthApplicable() {
    return false;
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, ShapeOfTheEarthType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void ShapeOfTheEarthOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                       OutDict_t& out) {
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (shape_of_the_earthApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept ShapeOfTheEarth] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(shape_of_the_earthTypeName<Variant>()) << std::endl;

            // Just do nothing for the moment
            return;
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(shapeOfTheEarthName), std::string(shape_of_the_earthTypeName<Variant>()),
                std::to_string(Stage), std::to_string(Section), "Unable to set `ensemble` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(shapeOfTheEarthName),
                                    std::string(shape_of_the_earthTypeName<Variant>()), std::to_string(Stage),
                                    std::to_string(Section), "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

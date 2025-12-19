#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/level/level_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsLevelist.h"
#include "metkit/mars2grib/backend/deductions/pvArray.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {


// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, LevelType Variant>
constexpr bool levelApplicable() {

    if constexpr (Section == SecProductDefinitionSection && Variant == LevelType::Hybrid) {
        // Hybrid needs to allocate space for the pv array
        return true;
    }

    if constexpr (Section == SecProductDefinitionSection && Variant != LevelType::Hybrid) {
        // Hybrid needs to allocate space for the pv array
        return (Stage != StageAllocate);
    }

    return false;
}

template <LevelType Variant>
constexpr bool needLevel() {
    if constexpr (Variant == LevelType::HeightAboveGroundAt10M || Variant == LevelType::HeightAboveGroundAt2M ||
                  Variant == LevelType::HeightAboveGround || Variant == LevelType::HeightAboveSeaAt10M ||
                  Variant == LevelType::HeightAboveSeaAt2M || Variant == LevelType::HeightAboveSea ||
                  Variant == LevelType::Hybrid || Variant == LevelType::IsobaricInHpa ||
                  Variant == LevelType::IsobaricInPa || Variant == LevelType::Isothermal ||
                  Variant == LevelType::PotentialVorticity || Variant == LevelType::SeaIceLayer ||
                  Variant == LevelType::SnowLayer || Variant == LevelType::SoilLayer || Variant == LevelType::Theta) {
        return true;
    }
    else {
        return false;
    }

    // Remove compiler warning
    __builtin_unreachable();
}


template <LevelType Variant>
constexpr bool needPv() {
    if constexpr (Variant == LevelType::Hybrid) {
        return true;
    }
    else {
        return false;
    }

    // Remove compiler warning
    __builtin_unreachable();
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, LevelType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void LevelOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (levelApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Level] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(levelTypeName<Variant>()) << std::endl;

            if constexpr (Stage == StageAllocate && needPv<Variant>()) {

                // Allocate space for pv array
                std::vector<double> pv_array = deductions::pvArray(mars, par);

                // Set the PV array
                set_or_throw<long>(out, "PVPresent", 1L);
                set_or_throw<std::vector<double>>(out, "pv", pv_array);
            }

            if constexpr (Stage == StagePreset || Stage == StageRuntime) {

                // Set level type (and level)
                if constexpr (Variant == LevelType::HeightAboveGroundAt2M) {
                    set_or_throw<std::string>(out, "typeOfLevel", "heightAboveGround");
                    set_or_throw<long>(out, "level", 2L);
                }
                else if constexpr (Variant == LevelType::HeightAboveGroundAt10M) {
                    set_or_throw<std::string>(out, "typeOfLevel", "heightAboveGround");
                    set_or_throw<long>(out, "level", 10L);
                }
                else if constexpr (Variant == LevelType::HeightAboveSeaAt2M) {
                    set_or_throw<std::string>(out, "typeOfLevel", "heightAboveSea");
                    set_or_throw<long>(out, "level", 2L);
                }
                else if constexpr (Variant == LevelType::HeightAboveSeaAt10M) {
                    set_or_throw<std::string>(out, "typeOfLevel", "heightAboveSea");
                    set_or_throw<long>(out, "level", 10L);
                }
                else {
                    set_or_throw<std::string>(out, "typeOfLevel", std::string(levelTypeName<Variant>()));
                    if constexpr (needLevel<Variant>()) {
                        long levelVal = deductions::marsLevelist(mars, par);
                        set_or_throw<long>(out, "level", levelVal);
                    }
                }
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(levelName), std::string(levelTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `ensemble` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(levelName), std::string(levelTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

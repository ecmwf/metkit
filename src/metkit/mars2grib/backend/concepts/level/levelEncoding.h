/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file levelOp.h
 * @brief Implementation of the GRIB `level` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **level concept** within the mars2grib backend.
 *
 * The level concept is responsible for encoding GRIB keys related to the
 * *vertical coordinate system* of the data, including:
 *
 * - `typeOfLevel`
 * - `level`
 * - hybrid vertical coordinate parameters (`pv` array)
 *
 * Depending on the selected level variant, the concept may:
 * - set only the level type,
 * - set both level type and numeric level,
 * - allocate and populate the PV array (hybrid levels).
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `levelApplicable`
 * - Stage-aware behavior (allocation vs preset/runtime)
 * - Explicit handling of hybrid vertical coordinates
 * - Strict error handling with contextual concept exceptions
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of `concepts`
 * to avoid ambiguity and potential conflicts with the C++20 `concept` language
 * feature and related standard headers.
 *
 * This is a deliberate design choice and must not be changed.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/level/levelEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/level.h"
#include "metkit/mars2grib/backend/deductions/pvArray.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time predicate indicating whether a PV array is required.
 *
 * Only hybrid vertical coordinates require a PV array describing the
 * vertical transformation.
 *
 * @tparam Variant Level concept variant
 *
 * @return `true` if a PV array is required,
 *         `false` otherwise.
 */
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

/**
 * @brief Compile-time predicate indicating whether a numeric `level` value is required.
 *
 * Some level types require an associated numeric level (e.g. pressure, height),
 * while others encode only the level type.
 *
 * @tparam Variant Level concept variant
 *
 * @return `true` if a numeric `level` value must be set,
 *         `false` otherwise.
 */
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


/**
 * @brief Compile-time applicability predicate for the `level` concept.
 *
 * This predicate determines whether the level concept is applicable for a given
 * combination of:
 * - encoding stage
 * - GRIB section
 * - level variant
 *
 * Applicability is evaluated entirely at compile time and is used by the
 * concept dispatcher to control instantiation and execution.
 *
 * Hybrid levels require special handling:
 * - during allocation stage to reserve space for the PV array,
 * - during preset/runtime stages to set the level type and parameters.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Level concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, LevelType Variant>
constexpr bool levelApplicable() {

    if constexpr (Section == SecProductDefinitionSection && needPv<Variant>()) {
        // pvArray needs to be allocated at allocation stage
        return true;
    }

    if constexpr (Section == SecProductDefinitionSection && !needPv<Variant>()) {
        return (Stage != StageAllocate);
    }

    return false;
}


/**
 * @brief Execute the `level` concept operation.
 *
 * This function implements the runtime logic of the GRIB `level` concept.
 * When applicable, it:
 *
 * - allocates and sets the PV array for hybrid levels during allocation stage,
 * - sets the GRIB `typeOfLevel` key,
 * - sets the numeric `level` key when required.
 *
 * The behavior is explicitly stage-dependent:
 * - `StageAllocate` is used for memory allocation (PV array),
 * - `StagePreset` and `StageRuntime` are used for semantic encoding.
 *
 * If the concept is invoked when not applicable, a
 * `Mars2GribConceptException` is thrown.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Level concept variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary (currently unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @param[in]  mars MARS input dictionary
 * @param[in]  geo  Geometry dictionary (currently unused)
 * @param[in]  par  Parameter dictionary
 * @param[in]  opt  Options dictionary
 * @param[out] out  Output GRIB dictionary to be populated
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
 *         If:
 *         - required deductions fail,
 *         - invalid stage/variant combinations are invoked,
 *         - any GRIB key cannot be set.
 *
 * @note
 * - All runtime errors are wrapped with full concept context
 *   (concept name, variant, stage, section).
 * - The concept does not rely on pre-existing GRIB header state.
 * - Se of typeOfLevel is happening at both preset and runtime stages because
 *   sometimes due to sideeffects in eccodes the typeOfLevel set at preset stage
 *   can be overwritten before runtime stage.
 *
 *
 * @see levelApplicable
 * @see needLevel
 * @see needPv
 */
template <std::size_t Stage, std::size_t Section, LevelType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void LevelOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (levelApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(level);

            // =============================================================
            // Allocation stage (PV array)
            // =============================================================
            if constexpr (Stage == StageAllocate && needPv<Variant>()) {

                // Allocate space for pv array
                std::vector<double> pv_array = deductions::resolve_PvArray_or_throw(mars, par, opt);

                // Set the PV array
                set_or_throw<long>(out, "PVPresent", 1L);
                set_or_throw<std::vector<double>>(out, "pv", pv_array);
            }


            // =============================================================
            // Preset / runtime stage
            // =============================================================
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
                else if constexpr (Variant == LevelType::IsobaricInHpa) {
                    long levelVal = deductions::resolve_Level_or_throw(mars, par, opt);
                    set_or_throw<std::string>(out, "typeOfLevel", "isobaricInhPa");
                    set_or_throw<long>(out, "level", levelVal / 100);
                }
                else {
                    set_or_throw<std::string>(out, "typeOfLevel", std::string(levelTypeName<Variant>()));
                    if constexpr (needLevel<Variant>()) {
                        long levelVal = deductions::resolve_Level_or_throw(mars, par, opt);
                        set_or_throw<long>(out, "level", levelVal);
                    }
                }
            }
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(level, "Unable to set `level` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(level, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

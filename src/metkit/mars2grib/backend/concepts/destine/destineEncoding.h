/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file destineOp.h
/// @brief Implementation of the GRIB `destine` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **DestinE concept** within the mars2grib backend.
///
/// The concept is responsible for populating GRIB keys in the
/// *Local Use Section* associated with **Destination Earth (DestinE) datasets**,
/// including:
///
/// - dataset identification (`dataset`)
/// - experiment metadata (`activity`, `experiment`)
/// - model and resolution identifiers
/// - ensemble realization and generation indices
///
/// The behavior of the concept depends on the selected DestinE variant:
///
/// - `DestineType::ClimateDT`
/// - `DestineType::ExtremesDT`
///
/// Each variant enforces strict dataset consistency and encodes a
/// different subset of metadata keys.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `destineApplicable`
/// - Runtime validation of Local Use Section constraints
/// - Delegation of semantic resolution to dedicated deduction functions
/// - Strict error handling with contextual concept exceptions
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of `concepts`
/// to avoid ambiguity and potential conflicts with the C++20 `concept` language
/// feature and related standard headers.
///
/// This is a deliberate design choice and must not be changed.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/destine/destineEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/activity.h"
#include "metkit/mars2grib/backend/deductions/experiment.h"
#include "metkit/mars2grib/backend/deductions/generation.h"
#include "metkit/mars2grib/backend/deductions/model.h"
#include "metkit/mars2grib/backend/deductions/realization.h"
#include "metkit/mars2grib/backend/deductions/resolution.h"

// checks
#include "metkit/mars2grib/backend/checks/checkDestinELocalSection.h"
#include "metkit/mars2grib/backend/checks/matchDataset.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `destine` concept.
///
/// This predicate determines whether the DestinE concept is applicable
/// for a given combination of:
/// - encoding stage
/// - GRIB section
/// - DestinE variant
///
/// Applicability is evaluated entirely at compile time and is used by the
/// concept dispatcher to ensure that only valid concept instantiations occur.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant DestinE concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
/// @note
/// The default applicability rule enables the concept only when:
/// - `Variant` is either `ClimateDT` or `ExtremesDT`
/// - `Stage == StagePreset`
/// - `Section == SecLocalUseSection`
///
/// This reflects the current DestinE GRIB encoding requirements.
///
template <std::size_t Stage, std::size_t Section, DestineType Variant>
constexpr bool destineApplicable() {
    // Confitions to apply concept
    return ((Variant == DestineType::ClimateDT || Variant == DestineType::ExtremesDT) && (Stage == StagePreset) &&
            (Section == SecLocalUseSection));
}


///
/// @brief Execute the DestinE concept operation.
///
/// This function implements the runtime logic of the GRIB `destine` concept.
/// When applicable, it:
///
/// 1. Validates that the Local Use Section is compatible with DestinE encoding.
/// 2. Enforces dataset consistency based on the selected DestinE variant.
/// 3. Deduces DestinE-specific metadata from MARS and parameter dictionaries.
/// 4. Encodes the corresponding GRIB keys in the output dictionary.
///
/// The concept supports two variants:
///
/// - **ExtremesDT**
/// - Enforces dataset `"extremes-dt"`
/// - Encodes only the dataset identifier
///
/// - **ClimateDT**
/// - Enforces dataset `"climate-dt"`
/// - Encodes activity, experiment, resolution, model,
/// generation, and realization metadata
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    DestinE concept variant
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the GRIB output dictionary
///
/// @param[in]  mars MARS input dictionary
/// @param[in]  par  Parameter dictionary
/// @param[in]  opt  Options dictionary
/// @param[out] out  Output GRIB dictionary to be populated
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
/// If:
/// - the Local Use Section does not match DestinE requirements
/// - dataset consistency checks fail
/// - any deduction fails
/// - any GRIB key cannot be set
///
/// @note
/// - All runtime errors are wrapped with full concept context
/// (concept name, variant, stage, section).
/// - This concept does not rely on any pre-existing GRIB header state.
///
/// @see destineApplicable
///
template <std::size_t Stage, std::size_t Section, DestineType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void DestineOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (destineApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(destine);

            // =============================================================
            // Structural validation
            // =============================================================
            validation::check_DestinELocalSection_or_throw(opt, out);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Variant == DestineType::ExtremesDT) {

                // Enforce dataset consistency
                validation::match_Dataset_or_throw(opt, out, "extremes-dt");

                // Encode dataset identifier
                set_or_throw<std::string>(out, "dataset", "extremes-dt");
            }
            else if constexpr (Variant == DestineType::ClimateDT) {

                // Enforce dataset consistency
                validation::match_Dataset_or_throw(opt, out, "climate-dt");

                // Encode dataset identifier
                set_or_throw<std::string>(out, "dataset", "climate-dt");

                // Deductions
                std::string activityVal   = deductions::resolve_Activity_or_throw(mars, par, opt);
                std::string experimentVal = deductions::resolve_Experiment_or_throw(mars, par, opt);
                std::string resolutionVal = deductions::resolve_Resolution_or_throw(mars, par, opt);
                std::string modelVal      = deductions::resolve_Model_or_throw(mars, par, opt);

                long generationVal  = deductions::resolve_Generation_or_throw(mars, par, opt);
                long realizationVal = deductions::resolve_Realization_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<std::string>(out, "activity", activityVal);
                set_or_throw<std::string>(out, "experiment", experimentVal);
                set_or_throw<std::string>(out, "resolution", resolutionVal);
                set_or_throw<std::string>(out, "model", modelVal);
                set_or_throw<long>(out, "generation", generationVal);
                set_or_throw<long>(out, "realization", realizationVal);
            }
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(destine, "Unable to set `destine` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(destine, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

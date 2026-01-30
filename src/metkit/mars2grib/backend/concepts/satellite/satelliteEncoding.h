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
 * @file satelliteOp.h
 * @brief Implementation of the GRIB `satellite` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **satellite concept**, which is responsible for encoding satellite-related
 * metadata into GRIB messages.
 *
 * The concept populates keys related to:
 * - satellite identification
 * - instrument characteristics
 * - spectral channel information
 *
 * The encoding is distributed across multiple GRIB sections and encoding
 * stages, reflecting the structure of GRIB Product Definition Templates
 * for satellite products.
 *
 * ---
 *
 * ## Encoding stages and sections
 *
 * The `satellite` concept operates in the following contexts:
 *
 * ### Local Use Section (Section 2)
 * - **StagePreset**
 *   - Encodes the `channel` key
 *   - Validates the Local Definition Number
 *
 * ### Product Definition Section (Section 4)
 * - **StageAllocate**
 *   - Allocates space for spectral band information
 *   - Sets `numberOfContributingSpectralBands`
 *
 * - **StagePreset**
 *   - Encodes satellite and instrument identifiers
 *   - Encodes spectral wave number information
 *
 * ---
 *
 * ## Supported Product Definition Templates
 *
 * The concept currently supports GRIB Product Definition Templates:
 *
 * - Template 32
 * - Template 33
 *
 * Any other template results in a validation error.
 *
 * ---
 *
 * ## Deductions
 *
 * All satellite-related values are obtained via dedicated deduction
 * functions, including:
 *
 * - channel
 * - satellite series and number
 * - instrument type
 * - spectral wave number scaling
 *
 * The concept does not perform implicit defaults or fallbacks.
 * Missing or inconsistent information results in a deduction error.
 *
 * ---
 *
 * ## Applicability model
 *
 * The satellite concept is applicable when **any** of the following holds:
 *
 * - `Stage == StagePreset`  and `Section == SecLocalUseSection`
 * - `Stage == StageAllocate` and `Section == SecProductDefinitionSection`
 * - `Stage == StagePreset`  and `Section == SecProductDefinitionSection`
 *
 * Variant-specific behavior (via `SatelliteType`) is currently not
 * differentiated and may be refined in future iterations.
 *
 * ---
 *
 * ## Error handling
 *
 * - Structural mismatches with GRIB templates are detected via validation helpers.
 * - All deduction and encoding errors are wrapped in a
 *   `Mars2GribConceptException` with full context.
 *
 * ---
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of `concepts`
 * to avoid conflicts with the C++20 `concept` language feature.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/satellite/satelliteEnum.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchLocalDefinitionNumber.h"
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/channel.h"
#include "metkit/mars2grib/backend/deductions/instrumentType.h"
#include "metkit/mars2grib/backend/deductions/satelliteNumber.h"
#include "metkit/mars2grib/backend/deductions/satelliteSeries.h"
#include "metkit/mars2grib/backend/deductions/scaleFactorOfCentralWaveNumber.h"
#include "metkit/mars2grib/backend/deductions/scaledValueOfCentralWaveNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `satellite` concept.
 *
 * This predicate determines whether the `satellite` concept is instantiated
 * for a given encoding stage and GRIB section.
 *
 * The concept is applicable in multiple stages and sections, reflecting
 * the distribution of satellite metadata across the GRIB message:
 *
 * - Local Use Section during preset
 * - Product Definition Section during allocation and preset
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Satellite concept variant
 *
 * @return `true` if the concept is applicable, `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, SatelliteType Variant>
constexpr bool satelliteApplicable() {

    bool condition1 = (Stage == StagePreset && Section == SecLocalUseSection);
    bool condition2 = (Stage == StageAllocate && Section == SecProductDefinitionSection);
    bool condition3 = (Stage == StagePreset && Section == SecProductDefinitionSection);


    return (condition1 || condition2 || condition3);
}

/**
 * @brief Execute the `satellite` concept operation.
 *
 * This function implements the runtime logic of the GRIB `satellite` concept.
 * Depending on the encoding stage and GRIB section, it:
 *
 * - validates structural constraints
 * - deduces satellite and instrument metadata
 * - encodes satellite-related GRIB keys
 *
 * The operation is stage- and section-aware and performs different actions
 * in each context.
 *
 * @tparam Stage      Encoding stage
 * @tparam Section    GRIB section index
 * @tparam Variant    Satellite concept variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @param[in]  mars MARS input dictionary
 * @param[in]  par  Parameter dictionary
 * @param[in]  opt  Options dictionary
 * @param[out] out  Output GRIB dictionary to be populated
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
 *         If:
 *         - the concept is called when not applicable
 *         - GRIB structural validation fails
 *         - any required deduction fails
 *         - encoding of satellite metadata fails
 *
 * @note
 * - The concept assumes that satellite products contribute exactly one
 *   spectral band.
 * - All values are explicitly set; no reliance on pre-existing GRIB state
 *   is permitted.
 *
 * @see satelliteApplicable
 */
template <std::size_t Stage, std::size_t Section, SatelliteType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void SatelliteOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (satelliteApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(satellite);

            if constexpr (Section == SecLocalUseSection && Stage == StagePreset) {

                // Check/Validation
                validation::match_LocalDefinitionNumber_or_throw(opt, out, {14});

                // Deductions
                long channel = deductions::resolve_Channel_or_throw(mars, par, opt);

                // Encoding
                set_or_throw(out, "channel", channel);
            }

            if constexpr (Section == SecProductDefinitionSection && Stage == StageAllocate) {

                // Check/Validation
                validation::match_ProductDefinitionTemplateNumber_or_throw(opt, out, {32, 33});

                // Encoding
                set_or_throw<long>(out, "numberOfContributingSpectralBands", 1L);
            }

            if constexpr (Section == SecProductDefinitionSection && Stage == StagePreset) {

                // Check/Validation
                validation::match_ProductDefinitionTemplateNumber_or_throw(opt, out, {32, 33});

                // Deductions
                long satelliteNumber = deductions::resolve_satelliteNumber_or_throw(mars, par, opt);
                long instrumentType  = deductions::resolve_InstrumentType_or_throw(mars, par, opt);

                long satelliteSeries = deductions::resolve_SatelliteSeries_or_throw(mars, par, opt);
                long scaleFactorOfCentralWaveNumber =
                    deductions::resolve_ScaleFactorOfCentralWaveNumber_or_throw(mars, par, opt);
                long scaledValueOfCentralWaveNumber =
                    deductions::resolve_ScaledValueOfCentralWaveNumber_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "satelliteSeries", satelliteSeries);
                set_or_throw<long>(out, "satelliteNumber", satelliteNumber);
                set_or_throw<long>(out, "instrumentType", instrumentType);
                set_or_throw<long>(out, "scaleFactorOfCentralWaveNumber", scaleFactorOfCentralWaveNumber);
                set_or_throw<long>(out, "scaledValueOfCentralWaveNumber", scaledValueOfCentralWaveNumber);
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(satellite, "Unable to set `satellite` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(satellite, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

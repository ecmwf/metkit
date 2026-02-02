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
 * @file waveOp.h
 * @brief Implementation of the GRIB `wave` concept.
 *
 * This header defines the applicability rules and execution logic for the
 * **wave concept** within the mars2grib backend.
 *
 * The `wave` concept is responsible for encoding GRIB keys related to
 * wave spectral and wave period metadata, depending on:
 * - the encoding stage,
 * - the GRIB section,
 * - the selected wave variant.
 *
 * The concept supports two distinct variants:
 * - `WaveType::Spectra`
 * - `WaveType::Period`
 *
 * Each variant is active at different stages of the encoding pipeline and
 * operates on different subsets of GRIB keys, as dictated by the GRIB2
 * Product Definition Templates (PDTs).
 *
 * The implementation follows the standard mars2grib concept pattern:
 * - Compile-time applicability via `waveApplicable`
 * - Strict validation against expected PDTs
 * - Variant- and stage-specific deductions
 * - Deterministic encoding into the output dictionary
 * - Context-rich error handling via concept exceptions
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of `concepts`
 * to avoid conflicts with the C++20 `concepts` language feature.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/wave/waveEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/periodItMax.h"
#include "metkit/mars2grib/backend/deductions/periodItMin.h"
#include "metkit/mars2grib/backend/deductions/waveDirectionGrid.h"
#include "metkit/mars2grib/backend/deductions/waveDirectionNumber.h"
#include "metkit/mars2grib/backend/deductions/waveFrequencyGrid.h"
#include "metkit/mars2grib/backend/deductions/waveFrequencyNumber.h"

// Tables
#include "metkit/mars2grib/backend/tables/typeOfInterval.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `wave` concept.
 *
 * This predicate determines whether the `wave` concept is applicable for a
 * given combination of:
 * - encoding stage,
 * - GRIB section,
 * - wave variant.
 *
 * The default applicability rules are:
 *
 * - **Spectral wave data**
 *   - `StageAllocate`, `SecProductDefinitionSection`, `WaveType::Spectra`
 *   - `StageRuntime`,  `SecProductDefinitionSection`, `WaveType::Spectra`
 *
 * - **Wave period data**
 *   - `StagePreset`,   `SecProductDefinitionSection`, `WaveType::Period`
 *
 * Any other combination is considered invalid and results in a runtime
 * concept error if invoked.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Wave concept variant
 *
 * @return `true` if the concept is applicable, `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, WaveType Variant>
constexpr bool waveApplicable() {
    bool condition1 =
        (Section == SecProductDefinitionSection && Stage == StageAllocate && Variant == WaveType::Spectra);

    bool condition2 = (Section == SecProductDefinitionSection && Stage == StagePreset && Variant == WaveType::Period);

    bool condition3 = (Section == SecProductDefinitionSection && Stage == StageRuntime && Variant == WaveType::Spectra);

    return condition1 || condition2 || condition3;
}


/**
 * @brief Execute the `wave` concept operation.
 *
 * This function implements the runtime logic for encoding wave-related
 * GRIB metadata. The behavior depends on both the wave variant and the
 * encoding stage.
 *
 * ---
 * ### Variant `WaveType::Spectra`
 *
 * #### StageAllocate
 * - Validates that the Product Definition Template Number is one of `{99, 100}`.
 * - Deduces and encodes:
 *   - Wave direction grid (number, scale factor, scaled values)
 *   - Wave frequency grid (number, scale factor, scaled values)
 *
 * #### StageRuntime
 * - Deduces and encodes:
 *   - `waveDirectionNumber`
 *   - `waveFrequencyNumber`
 *
 * ---
 * ### Variant `WaveType::Period`
 *
 * #### StagePreset
 * - Validates that the Product Definition Template Number is one of `{103, 104}`.
 * - Deduces optional lower and/or upper wave period bounds.
 * - Encodes wave period interval metadata according to the availability
 *   of minimum and/or maximum bounds.
 *
 * ---
 * ### Validation
 *
 * Each variant is validated against the expected GRIB Product Definition
 * Template Number(s) before any encoding is performed.
 *
 * ---
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Wave concept variant
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
 *         - the concept is invoked outside its applicability domain,
 *         - the Product Definition Template Number does not match expectations,
 *         - any wave grid or wave period deduction fails,
 *         - any encoding operation fails.
 *
 * @note
 * For `WaveType::Period`, the code explicitly notes that some fields are
 * already implicitly set by ecCodes via `paramId`. Overwriting these values
 * may be redundant and should be reviewed once the final wave-period
 * encoding policy is agreed.
 *
 * @warning
 * If neither `periodItMin` nor `periodItMax` is present, no wave-period
 * interval metadata is written. This is currently allowed but may require
 * stricter validation in the future.
 *
 * @see waveApplicable
 * @see deductions::resolve_WaveDirectionGrid_or_throw
 * @see deductions::resolve_WaveFrequencyGrid_or_throw
 * @see deductions::resolve_WaveDirectionNumber_or_throw
 * @see deductions::resolve_WaveFrequencyNumber_or_throw
 * @see deductions::resolve_PeriodItMin_opt
 * @see deductions::resolve_PeriodItMax_opt
 */
template <std::size_t Stage, std::size_t Section, WaveType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
          class OutDict_t>
void WaveOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (waveApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(wave);

            // Checks/Validation
            if constexpr (Variant == WaveType::Spectra) {
                validation::match_ProductDefinitionTemplateNumber_or_throw(opt, out, {99, 100});
            }
            else if constexpr (Variant == WaveType::Period) {
                validation::match_ProductDefinitionTemplateNumber_or_throw(opt, out, {103, 104});
            }

            if constexpr (Stage == StageAllocate) {

                if constexpr (Variant == WaveType::Spectra) {

                    // Deductions
                    deductions::WaveDirectionGrid directionGrid =
                        deductions::resolve_WaveDirectionGrid_or_throw(mars, par, opt);

                    deductions::WaveFrequencyGrid frequencyGrid =
                        deductions::resolve_WaveFrequencyGrid_or_throw(mars, par, opt);


                    // Encoding
                    set_or_throw<long>(out, "numberOfWaveDirections", directionGrid.numDirections);
                    set_or_throw<long>(out, "scaleFactorOfWaveDirections", directionGrid.scaleFactorDirections);
                    set_or_throw<std::vector<long>>(out, "scaledValuesOfWaveDirections",
                                                    directionGrid.scaledValuesDirections);

                    set_or_throw<long>(out, "numberOfWaveFrequencies", frequencyGrid.numFrequencies);
                    set_or_throw<long>(out, "scaleFactorOfWaveFrequencies", frequencyGrid.scaleFactorFrequencies);
                    set_or_throw<std::vector<long>>(out, "scaledValuesOfWaveFrequencies",
                                                    frequencyGrid.scaledValuesFrequencies);
                }
            }

            if constexpr (Stage == StagePreset) {

                if constexpr (Variant == WaveType::Period) {

                    // Deductions
                    std::optional<long> itMin = deductions::resolve_PeriodItMin_opt(mars, par, opt);
                    std::optional<long> itMax = deductions::resolve_PeriodItMax_opt(mars, par, opt);

                    // Encoding
                    /// @note:
                    /// - This information is set by eccodes as part of the paramId, not really
                    ///   sure it make sense to (over)write it here...
                    if (itMin.has_value() && itMax.has_value()) {
                        set_or_throw<long>(
                            out, "typeOfWavePeriodInterval",
                            static_cast<long>(tables::TypeOfInterval::BetweenFirstInclusiveSecondInclusive));
                        set_or_throw<long>(out, "scaleFactorOfLowerWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfLowerWavePeriodLimit", itMin.value());
                        set_or_throw<long>(out, "scaleFactorOfUpperWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfUpperWavePeriodLimit", itMax.value());
                    }
                    else if (itMin.has_value() && !itMax.has_value()) {
                        set_or_throw<long>(out, "typeOfWavePeriodInterval",
                                           static_cast<long>(tables::TypeOfInterval::GreaterThanFirstLimit));
                        set_or_throw<long>(out, "scaleFactorOfLowerWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfLowerWavePeriodLimit", itMin.value());
                    }
                    else if (!itMin.has_value() && itMax.has_value()) {
                        set_or_throw<long>(out, "typeOfWavePeriodInterval",
                                           static_cast<long>(tables::TypeOfInterval::SmallerThanSecondLimit));
                        set_or_throw<long>(out, "scaleFactorOfUpperWavePeriodLimit", 0L);
                        set_or_throw<long>(out, "scaledValueOfUpperWavePeriodLimit", itMax.value());
                    }

                }  // if constexpr ( Variant == WaveType::Period )
            }

            if constexpr (Stage == StageRuntime) {

                if constexpr (Variant == WaveType::Spectra) {

                    // Deduction
                    long marsDir  = deductions::resolve_WaveDirectionNumber_or_throw(mars, par, opt);
                    long marsFreq = deductions::resolve_WaveFrequencyNumber_or_throw(mars, par, opt);

                    // Encoding
                    set_or_throw<long>(out, "waveDirectionNumber", marsDir);
                    set_or_throw<long>(out, "waveFrequencyNumber", marsFreq);
                }
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(wave, "Unable to set `wave` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(wave, "Concept called when not applicable...");


    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file referencePeriodEncoding.h
/// @brief Implementation of the GRIB `referencePeriod` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **referencePeriod concept** within the mars2grib backend.
///
/// The concept encodes the reference-period block of PDT 4.106/4.107/4.112/
/// 4.131-4.135: the additional parameters (SOT percentiles), the model-climate
/// time ranges and sample size, and for EFI/SOT the reference dataset and
/// relation.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `referencePeriodApplicable`
/// - Runtime execution via `ReferencePeriodOp`
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
#include <array>
#include <string>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/reference-period/referencePeriodEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/additionalParametersForReferencePeriod.h"
#include "metkit/mars2grib/backend/deductions/modelClimateReferencePeriod.h"
#include "metkit/mars2grib/backend/deductions/sampleSizeOfModelClimate.h"
#include "metkit/mars2grib/backend/deductions/startOfReferencePeriod.h"
#include "metkit/mars2grib/backend/deductions/typeOfReferenceDataset.h"
#include "metkit/mars2grib/backend/deductions/typeOfRelationToReferenceDataset.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `referencePeriod` concept.
///
/// The concept is applicable in the allocate and preset stages of the Product Definition Section.
///
template <std::size_t Stage, std::size_t Section, ReferencePeriodType Variant>
constexpr bool referencePeriodApplicable() {
    return (Section == SecProductDefinitionSection) && (Stage == StageAllocate || Stage == StagePreset);
}

///
/// @brief Execute the `referencePeriod` concept operation.
///
/// StageAllocate:
/// - `numberOfAdditionalParametersForReferencePeriod` (2 for SOT, else 0)
/// - `numberOfReferencePeriodTimeRanges` (2)
///
/// StagePreset:
/// - `typeOfReferenceDataset` (code table 4.100) and `typeOfRelationToReferenceDataset` (code table 4.101), see
///   `resolve_TypeOfReferenceDataset_or_throw` and `resolve_TypeOfRelationToReferenceDataset_or_throw`
/// - SOT only: the two percentiles as additional parameters (`scaleFactorOfAdditionalParameterForReferencePeriod`,
///   `scaledValueOfAdditionalParameterForReferencePeriod`), see
///   `resolve_AdditionalParametersForReferencePeriod_or_throw`
/// - the start of the reference period (`yearOfStartOfReferencePeriod`, `monthOfStartOfReferencePeriod`,
///   `dayOfStartOfReferencePeriod`, `hourOfStartOfReferencePeriod`, `minuteOfStartOfReferencePeriod`,
///   `secondOfStartOfReferencePeriod`), only set when resolved, see `resolve_StartOfReferencePeriod_opt`
/// - `sampleSizeOfReferencePeriod` from the optional `sampleSizeOfModelClimate`, only set when resolved, see
///   `resolve_SampleSizeOfModelClimate_opt`
/// - the model-climate time ranges (`typeOfStatisticalProcessingForTimeRangeForReferencePeriod`,
///   `indicatorOfUnitForTimeRangeForReferencePeriod`, `lengthOfTimeRangeForReferencePeriod`), outer (years) first
///   and inner (days) second, only set when resolved, see `resolve_ModelClimateReferencePeriod_opt`
///
template <std::size_t Stage, std::size_t Section, ReferencePeriodType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ReferencePeriodOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (referencePeriodApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(referencePeriod);


            // =============================================================
            // StageAllocate
            // =============================================================
            if constexpr (Stage == StageAllocate) {
                if constexpr (Variant == ReferencePeriodType::Quantile) {
                    set_or_throw<long>(out, "numberOfAdditionalParametersForReferencePeriod", 2);
                }
                else {
                    set_or_throw<long>(out, "numberOfAdditionalParametersForReferencePeriod", 0);
                }
                set_or_throw<long>(out, "numberOfReferencePeriodTimeRanges", 2);
            }

            // =============================================================
            // StagePreset
            // =============================================================
            if constexpr (Stage == StagePreset) {

                const auto typeOfReferenceDataset = deductions::resolve_TypeOfReferenceDataset_or_throw(mars, par, opt);
                set_or_throw<long>(out, "typeOfReferenceDataset", static_cast<long>(typeOfReferenceDataset));

                const auto typeOfRelationToReferenceDataset =
                    deductions::resolve_TypeOfRelationToReferenceDataset_or_throw(mars, par, opt);
                set_or_throw<long>(out, "typeOfRelationToReferenceDataset",
                                   static_cast<long>(typeOfRelationToReferenceDataset));

                if constexpr (Variant == ReferencePeriodType::Quantile) {
                    const auto additionalParameters =
                        deductions::resolve_AdditionalParametersForReferencePeriod_or_throw(mars, par, opt);

                    set_or_throw<std::vector<long>>(
                        out, "scaleFactorOfAdditionalParameterForReferencePeriod",
                        {additionalParameters.first.scaleFactor, additionalParameters.second.scaleFactor});
                    set_or_throw<std::vector<long>>(
                        out, "scaledValueOfAdditionalParameterForReferencePeriod",
                        {additionalParameters.first.scaledValue, additionalParameters.second.scaledValue});
                }

                const auto startOfReferencePeriod = deductions::resolve_StartOfReferencePeriod_opt(mars, par, opt);
                if (startOfReferencePeriod.has_value()) {
                    set_or_throw<long>(out, "yearOfStartOfReferencePeriod", startOfReferencePeriod->year);
                    set_or_throw<long>(out, "monthOfStartOfReferencePeriod", startOfReferencePeriod->month);
                    set_or_throw<long>(out, "dayOfStartOfReferencePeriod", startOfReferencePeriod->day);
                    set_or_throw<long>(out, "hourOfStartOfReferencePeriod", startOfReferencePeriod->hour);
                    set_or_throw<long>(out, "minuteOfStartOfReferencePeriod", startOfReferencePeriod->minute);
                    set_or_throw<long>(out, "secondOfStartOfReferencePeriod", startOfReferencePeriod->second);
                }

                const auto sampleSizeOfModelClimate = deductions::resolve_SampleSizeOfModelClimate_opt(mars, par, opt);
                if (sampleSizeOfModelClimate.has_value()) {
                    set_or_throw<long>(out, "sampleSizeOfReferencePeriod", sampleSizeOfModelClimate.value());
                }

                const auto modelClimate = deductions::resolve_ModelClimateReferencePeriod_opt(mars, par, opt);
                if (modelClimate.has_value()) {
                    set_or_throw<std::vector<long>>(
                        out, "typeOfStatisticalProcessingForTimeRangeForReferencePeriod",
                        {modelClimate->outer.typeOfStatisticalProcessingForTimeRangeForReferencePeriod,
                         modelClimate->inner.typeOfStatisticalProcessingForTimeRangeForReferencePeriod});
                    set_or_throw<std::vector<long>>(
                        out, "indicatorOfUnitForTimeRangeForReferencePeriod",
                        {modelClimate->outer.indicatorOfUnitForTimeRangeForReferencePeriod,
                         modelClimate->inner.indicatorOfUnitForTimeRangeForReferencePeriod});
                    set_or_throw<std::vector<long>>(out, "lengthOfTimeRangeForReferencePeriod",
                                                    {modelClimate->outer.lengthOfTimeRangeForReferencePeriod,
                                                     modelClimate->inner.lengthOfTimeRangeForReferencePeriod});
                }
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(referencePeriod, "Unable to set `referencePeriod` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(referencePeriod, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

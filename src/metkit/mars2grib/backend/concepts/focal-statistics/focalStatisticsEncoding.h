// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file focalStatisticsEncoding.h
/// @brief Implementation of the GRIB `focalStatistics` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **focal statistics concept** within the mars2grib backend.
///
/// The concept encodes the spatio-temporal window (vicinity) over which a
/// product is computed, in the *Product Definition Section*:
///
/// - `spatialVicinityType`
/// - `numberOfSpatialVicinityValues`
/// - `spatialVicinityValue`
/// - `spatialVicinityProcessing`
/// - `spatialVicinityProcessingArgument1`, `spatialVicinityProcessingArgument2`
/// - `spatialVicinityMissingData`
/// - `temporalVicinityProcessing`
/// - `temporalVicinityUnit`
/// - `temporalVicinityTowardsPast`
/// - `temporalVicinityTowardsFuture`
///
/// The spatial and temporal vicinity keys that depend on the product are
/// deduced from the paramId, with no user override. The following are fixed:
/// one spatial vicinity value, no processing arguments (missing), spatial
/// missing data handling `0`, temporal unit `1` (hours) and a temporal vicinity
/// towards the future of `0`.
///
/// The concept currently supports a single **Default** variant, valid for the
/// large-ensemble product definition templates (PDT 4.121/4.122).
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
#include "metkit/mars2grib/backend/concepts/focal-statistics/focalStatisticsEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/spatialVicinityProcessing.h"
#include "metkit/mars2grib/backend/deductions/spatialVicinityType.h"
#include "metkit/mars2grib/backend/deductions/spatialVicinityValue.h"
#include "metkit/mars2grib/backend/deductions/temporalVicinityProcessing.h"
#include "metkit/mars2grib/backend/deductions/temporalVicinityTowardsPastInHours.h"

// Tables
#include "metkit/mars2grib/backend/tables/spatialVicinityType.h"
#include "metkit/mars2grib/backend/tables/vicinityProcessing.h"

// checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `focalStatistics` concept.
///
/// The concept is applicable only when:
/// - `Stage == StagePreset`
/// - `Section == SecProductDefinitionSection`
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Focal statistics concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
template <std::size_t Stage, std::size_t Section, FocalStatisticsType Variant>
constexpr bool focalStatisticsApplicable() {
    return Stage == StagePreset && Section == SecProductDefinitionSection;
}


///
/// @brief Execute the `focalStatistics` concept operation.
///
/// When applicable, it:
///
/// 1. Validates that the Product Definition Template Number is 121 or 122.
/// 2. Deduces the paramId-dependent vicinity keys.
/// 3. Encodes the vicinity keys in the output dictionary.
/// 4. Sets the lower and upper limits of the template to missing.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Focal statistics concept variant
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
/// - the Product Definition Template Number is not compatible
/// - any deduction fails (including a paramId that is not supported)
/// - any GRIB key cannot be set
///
template <std::size_t Stage, std::size_t Section, FocalStatisticsType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void FocalStatisticsOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt,
                       OutDict_t& out) noexcept(false) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (focalStatisticsApplicable<Stage, Section, Variant>()) {

        try {

            // Debug output
            MARS2GRIB_LOG_CONCEPT(focalStatistics);

            // Structural validation
            validation::match_ProductDefinitionTemplateNumber_or_throw(opt, out, {121L, 122L});

            // Deductions
            const auto spatialVicinityType  = deductions::resolve_SpatialVicinityType_or_throw(mars, par, opt);
            const auto spatialVicinityValue = deductions::resolve_SpatialVicinityValue_or_throw(mars, par, opt);
            const auto spatialVicinityProcessing =
                deductions::resolve_SpatialVicinityProcessing_or_throw(mars, par, opt);
            const auto temporalVicinityProcessing =
                deductions::resolve_TemporalVicinityProcessing_or_throw(mars, par, opt);
            const auto temporalVicinityTowardsPastInHours =
                deductions::resolve_TemporalVicinityTowardsPastInHours_or_throw(mars, par, opt);

            // Fixed assumptions: a single spatial vicinity value, no processing arguments, hours as temporal unit
            // (Code Table 4.4: 1 = hour) and no vicinity towards the future
            constexpr long numberOfSpatialVicinityValues = 1L;
            // spatialVicinityProcessingArgument = missing
            constexpr long spatialVicinityMissingData         = 0L;
            constexpr long temporalVicinityUnitHours          = 1L;
            constexpr long temporalVicinityTowardsFutureHours = 0L;

            // Encoding (numberOfSpatialVicinityValues must be set before spatialVicinityValue)
            set_or_throw<long>(out, "spatialVicinityType", static_cast<long>(spatialVicinityType));
            set_or_throw<long>(out, "numberOfSpatialVicinityValues", numberOfSpatialVicinityValues);
            set_or_throw<long>(out, "spatialVicinityValue", spatialVicinityValue);
            set_or_throw<long>(out, "spatialVicinityProcessing", static_cast<long>(spatialVicinityProcessing));
            setMissing_or_throw(out, "spatialVicinityProcessingArgument1");
            setMissing_or_throw(out, "spatialVicinityProcessingArgument2");
            set_or_throw<long>(out, "spatialVicinityMissingData", spatialVicinityMissingData);
            set_or_throw<long>(out, "temporalVicinityProcessing", static_cast<long>(temporalVicinityProcessing));
            set_or_throw<long>(out, "temporalVicinityUnit", temporalVicinityUnitHours);
            set_or_throw<long>(out, "temporalVicinityTowardsPast", temporalVicinityTowardsPastInHours);
            set_or_throw<long>(out, "temporalVicinityTowardsFuture", temporalVicinityTowardsFutureHours);

            // The large-ensemble templates (PDT 4.121/4.122) also carry the lower/upper limits, which are not
            // defined by the paramId: set them to missing
            setMissing_or_throw(out, "scaleFactorOfLowerLimit");
            setMissing_or_throw(out, "scaledValueOfLowerLimit");
            setMissing_or_throw(out, "scaleFactorOfUpperLimit");
            setMissing_or_throw(out, "scaledValueOfUpperLimit");
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(focalStatistics, "Unable to set `focalStatistics` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(focalStatistics, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

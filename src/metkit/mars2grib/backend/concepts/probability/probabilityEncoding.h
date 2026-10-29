// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file probabilityEncoding.h
/// @brief Implementation of the GRIB `probability` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **probability concept** within the mars2grib backend.
///
/// The probability concept is responsible for encoding GRIB keys related to
/// ensemble probability products in the *Product Definition Section*:
///
/// - `forecastProbabilityNumber`
/// - `totalNumberOfForecastProbabilities`
/// - `probabilityType`
///
/// `forecastProbabilityNumber` (default 0) and
/// `totalNumberOfForecastProbabilities` (default 1) can be overridden through
/// the parameter dictionary (`misc`). The two deductions are independent of
/// each other: the consistency check
/// `forecastProbabilityNumber < totalNumberOfForecastProbabilities` is performed
/// here, where the keys are set. `probabilityType` is deduced from the paramId.
///
/// The concept currently supports a single **Default** variant.
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
#include "metkit/mars2grib/backend/concepts/probability/probabilityEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/forecastProbabilityNumber.h"
#include "metkit/mars2grib/backend/deductions/probabilityType.h"
#include "metkit/mars2grib/backend/deductions/totalNumberOfForecastProbabilities.h"

// Tables
#include "metkit/mars2grib/backend/tables/probabilityType.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `probability` concept.
///
/// The concept is applicable only when:
/// - `Stage == StagePreset`
/// - `Section == SecProductDefinitionSection`
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Probability concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
template <std::size_t Stage, std::size_t Section, ProbabilityType Variant>
constexpr bool probabilityApplicable() {
    return Stage == StagePreset && Section == SecProductDefinitionSection;
}


///
/// @brief Execute the `probability` concept operation.
///
/// When applicable, it:
///
/// 1. Deduces `forecastProbabilityNumber`, `totalNumberOfForecastProbabilities`
///    and `probabilityType`.
/// 2. Checks that `forecastProbabilityNumber < totalNumberOfForecastProbabilities`.
/// 3. Encodes the corresponding GRIB keys in the output dictionary.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Probability concept variant
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
/// - any deduction fails (including a paramId without a `probabilityType`)
/// - `forecastProbabilityNumber` is not less than `totalNumberOfForecastProbabilities`
/// - any GRIB key cannot be set
///
template <std::size_t Stage, std::size_t Section, ProbabilityType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ProbabilityOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) noexcept(false) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (probabilityApplicable<Stage, Section, Variant>()) {

        try {

            // Debug output
            MARS2GRIB_LOG_CONCEPT(probability);

            // Deductions (the two probability numbers are independent of each other)
            const auto forecastProbabilityNumber =
                deductions::resolve_ForecastProbabilityNumber_or_throw(mars, par, opt);
            const auto totalNumberOfForecastProbabilities =
                deductions::resolve_TotalNumberOfForecastProbabilities_or_throw(mars, par, opt);
            const auto probabilityType = deductions::resolve_ProbabilityType_or_throw(mars, par, opt);

            // Consistency check on the values being set
            if (!(forecastProbabilityNumber < totalNumberOfForecastProbabilities)) {
                std::string errMsg = "`forecastProbabilityNumber` (";
                errMsg += std::to_string(forecastProbabilityNumber);
                errMsg += ") must be less than `totalNumberOfForecastProbabilities` (";
                errMsg += std::to_string(totalNumberOfForecastProbabilities);
                errMsg += ")";
                MARS2GRIB_CONCEPT_THROW(probability, errMsg);
            }

            // Encoding
            set_or_throw<long>(out, "forecastProbabilityNumber", forecastProbabilityNumber);
            set_or_throw<long>(out, "totalNumberOfForecastProbabilities", totalNumberOfForecastProbabilities);
            set_or_throw<long>(out, "probabilityType", static_cast<long>(probabilityType));
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(probability, "Unable to set `probability` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(probability, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

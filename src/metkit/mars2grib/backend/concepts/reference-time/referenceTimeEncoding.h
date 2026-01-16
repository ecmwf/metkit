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
 * @file referenceTimeOp.h
 * @brief Implementation of the GRIB `referenceTime` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **referenceTime concept** within the mars2grib backend.
 *
 * The referenceTime concept is responsible for encoding the temporal
 * *reference instant* of a GRIB product, including:
 *
 * - the **significance of the reference time**
 * - the **actual reference date/time**
 * - optional **model version date/time** metadata for reforecasts
 *
 * The behavior depends on both the **concept variant** and the **GRIB section**
 * in which the concept is applied.
 *
 * Supported variants:
 * - `ReferenceTimeType::Standard`
 * - `ReferenceTimeType::Reforecast`
 *
 * Supported sections:
 * - Identification Section
 * - Product Definition Section (reforecast only)
 *
 * The implementation follows the standard mars2grib concept model:
 * - Compile-time applicability via `referenceTimeApplicable`
 * - Variant- and section-specific deduction logic
 * - Strict validation of GRIB template compatibility
 * - Context-rich error handling
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

// External libraries
#include <eckit/types/DateTime.h>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/hindcastDateTime.h"
#include "metkit/mars2grib/backend/deductions/referenceDateTime.h"
#include "metkit/mars2grib/backend/deductions/significanceOfReferenceTime.h"

// Tables
#include "metkit/mars2grib/backend/tables/significanceOfReferenceTime.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `referenceTime` concept.
 *
 * This predicate determines whether the referenceTime concept is applicable
 * for a given combination of:
 * - encoding stage
 * - GRIB section
 * - reference time variant
 *
 * The default rules enable:
 *
 * - **Standard reference time**
 *   - Identification Section
 *   - Preset stage
 *
 * - **Reforecast reference time**
 *   - Identification Section (Preset stage)
 *   - Product Definition Section (Preset stage)
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Reference time concept variant
 *
 * @return `true` if the concept is applicable for the given parameters,
 *         `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, ReferenceTimeType Variant>
constexpr bool referenceTimeApplicable() {

    // Compile time conditions to apply this concept
    bool condition1 = (Variant == ReferenceTimeType::Standard || Variant == ReferenceTimeType::Reforecast) &&
                      (Stage == StagePreset) && (Section == SecIdentificationSection);

    bool condition2 = (Variant == ReferenceTimeType::Reforecast) && (Stage == StagePreset) &&
                      (Section == SecProductDefinitionSection);

    // Confitions to apply concept
    return condition1 || condition2;
}


/**
 * @brief Execute the `referenceTime` concept operation.
 *
 * This function implements the runtime logic of the GRIB `referenceTime` concept.
 * When applicable, it:
 *
 * - Encodes the significance of the reference time
 * - Sets the reference date and time fields
 * - Optionally encodes model version date/time metadata for reforecasts
 *
 * The behavior is driven by:
 * - the concept variant (`Standard` vs `Reforecast`)
 * - the target GRIB section
 *
 * Section-specific behavior:
 *
 * - **Identification Section**
 *   - Sets `significanceOfReferenceTime`
 *   - Sets reference date/time fields
 *
 * - **Product Definition Section (Reforecast only)**
 *   - Validates template compatibility
 *   - Sets model version date/time fields
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Reference time concept variant
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
 *         - the concept is invoked outside its applicability domain
 *         - GRIB template constraints are violated
 *         - any deduction or encoding step fails
 *
 * @see referenceTimeApplicable
 */
template <std::size_t Stage, std::size_t Section, ReferenceTimeType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void ReferenceTimeOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                     OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (referenceTimeApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(referenceTime);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Section == SecIdentificationSection) {

                // Deductions
                tables::SignificanceOfReferenceTime significanceOfReferenceTime =
                    deductions::resolve_SignificanceOfReferenceTime_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "significanceOfReferenceTime", static_cast<long>(significanceOfReferenceTime));
            }

            if constexpr ((Section == SecIdentificationSection) && (Variant == ReferenceTimeType::Standard)) {

                // Deductions
                eckit::DateTime dateTime = deductions::resolve_ReferenceDateTime_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "year", dateTime.date().year());
                set_or_throw<long>(out, "month", dateTime.date().month());
                set_or_throw<long>(out, "day", dateTime.date().day());
                set_or_throw<long>(out, "hour", dateTime.time().hours());
                set_or_throw<long>(out, "minute", dateTime.time().minutes());
                set_or_throw<long>(out, "second", dateTime.time().seconds());
            }

            if constexpr ((Section == SecIdentificationSection) && (Variant == ReferenceTimeType::Reforecast)) {

                // Deductions
                eckit::DateTime referenceDateTime = deductions::resolve_HindcastDateTime_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "year", referenceDateTime.date().year());
                set_or_throw<long>(out, "month", referenceDateTime.date().month());
                set_or_throw<long>(out, "day", referenceDateTime.date().day());
                set_or_throw<long>(out, "hour", referenceDateTime.time().hours());
                set_or_throw<long>(out, "minute", referenceDateTime.time().minutes());
                set_or_throw<long>(out, "second", referenceDateTime.time().seconds());
            }

            if constexpr ((Section == SecProductDefinitionSection) && (Variant == ReferenceTimeType::Reforecast)) {

                // Validation
                validation::match_ProductDefinitionTemplateNumber_or_throw(opt, out, {60L, 61L});


                // Deduction
                eckit::DateTime dateTime = deductions::resolve_ReferenceDateTime_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "YearOfModelVersion", dateTime.date().year());
                set_or_throw<long>(out, "MonthOfModelVersion", dateTime.date().month());
                set_or_throw<long>(out, "DayOfModelVersion", dateTime.date().day());
                set_or_throw<long>(out, "HourOfModelVersion", dateTime.time().hours());
                set_or_throw<long>(out, "MinuteOfModelVersion", dateTime.time().minutes());
                set_or_throw<long>(out, "SecondOfModelVersion", dateTime.time().seconds());
            }
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(referenceTime, "Unable to set `referenceTime` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(referenceTime, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

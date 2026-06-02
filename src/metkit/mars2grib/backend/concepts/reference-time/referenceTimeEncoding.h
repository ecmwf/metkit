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
/// @file referenceTimeEncoding.h
/// @brief Implementation of the GRIB `referenceTime` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **referenceTime concept** within the mars2grib backend.
///
/// The referenceTime concept is responsible for encoding the temporal
/// *reference instant* of a GRIB product, including:
///
/// - the **significance of the reference time**
/// - the **actual reference date/time**
/// - optional **model version date/time** metadata for reforecasts
///
/// The behavior depends on both the **concept variant** and the **GRIB section**
/// in which the concept is applied.
///
/// Supported variants:
/// - `ReferenceTimeType::Standard`
/// - `ReferenceTimeType::Reforecast`
///
/// Supported sections:
/// - Identification Section
/// - Product Definition Section (reforecast only)
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `referenceTimeApplicable`
/// - Variant- and section-specific deduction logic
/// - Strict validation of GRIB template compatibility
/// - Context-rich error handling
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

// External libraries
#include <eckit/types/DateTime.h>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/reference-time/referenceTimeEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/productTime.h"
#include "metkit/mars2grib/backend/deductions/significanceOfReferenceTime.h"

// Tables
#include "metkit/mars2grib/backend/tables/significanceOfReferenceTime.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `referenceTime` concept.
///
/// This predicate determines whether the referenceTime concept is applicable
/// for a given combination of:
/// - encoding stage
/// - GRIB section
/// - reference time variant
///
/// The default rules enable:
///
/// - **Standard reference time**
/// - Identification Section
/// - Preset stage
///
/// - **Reforecast reference time**
/// - Identification Section (Preset stage)
/// - Product Definition Section (Preset stage)
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Reference time concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
template <std::size_t Stage, std::size_t Section, ReferenceTimeType Variant>
constexpr bool referenceTimeApplicable() {

    // Compile time conditions to apply this concept
    bool condition1 = (Variant == ReferenceTimeType::Standard || Variant == ReferenceTimeType::Reforecast) &&
                      (Stage == StageRuntime) && (Section == SecIdentificationSection);

    bool condition2 = (Variant == ReferenceTimeType::Reforecast) && (Stage == StageRuntime) &&
                      (Section == SecProductDefinitionSection);

    // Confitions to apply concept
    return condition1 || condition2;
}


///
/// @brief Execute the `referenceTime` concept operation.
///
/// This function implements the runtime logic of the GRIB `referenceTime` concept.
/// When applicable, it:
///
/// - Encodes the significance of the reference time
/// - Sets the reference date and time fields
/// - Optionally encodes model version date/time metadata for reforecasts
///
/// The behavior is driven by:
/// - the concept variant (`Standard` vs `Reforecast`)
/// - the target GRIB section
///
/// Section-specific behavior:
///
/// - **Identification Section**
/// - Sets `significanceOfReferenceTime`
/// - Sets reference date/time fields
///
/// - **Product Definition Section (Reforecast only)**
/// - Validates template compatibility
/// - Sets model version date/time fields
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Reference time concept variant
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
/// - the concept is invoked outside its applicability domain
/// - GRIB template constraints are violated
/// - any deduction or encoding step fails
///
/// @see referenceTimeApplicable
///
template <std::size_t Stage, std::size_t Section, ReferenceTimeType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ReferenceTimeOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (referenceTimeApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(referenceTime);

            // Resolve the canonical ProductTime once per concept invocation.
            // All three branches below source their date/time exclusively
            // from this object (§15 of timeProducts.md).
            auto pt = deductions::resolve_ProductTime_or_throw(mars, par, opt);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Section == SecIdentificationSection) {

                // Deductions: significanceOfReferenceTime is orthogonal to
                // ProductTime (driven by mars.type / mars.stream).
                tables::SignificanceOfReferenceTime significanceOfReferenceTime =
                    deductions::resolve_SignificanceOfReferenceTime_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "significanceOfReferenceTime", static_cast<long>(significanceOfReferenceTime));
            }

            if constexpr ((Section == SecIdentificationSection) && (Variant == ReferenceTimeType::Standard)) {

                // For a standard product, the GRIB "reference date/time"
                // is the canonical ProductTime::referenceDateTime.
                const eckit::DateTime& dateTime = pt.referenceDateTime;

                // Encoding
                set_or_throw<long>(out, "year", dateTime.date().year());
                set_or_throw<long>(out, "month", dateTime.date().month());
                set_or_throw<long>(out, "day", dateTime.date().day());
                set_or_throw<long>(out, "hour", dateTime.time().hours());
                set_or_throw<long>(out, "minute", dateTime.time().minutes());
                set_or_throw<long>(out, "second", dateTime.time().seconds());
            }

            if constexpr ((Section == SecIdentificationSection) && (Variant == ReferenceTimeType::Reforecast)) {

                // For a reforecast product, the Identification Section's
                // reference date/time is the hindcast date — i.e. the
                // canonical ProductTime::labelDateTime (from date /
                // time). The ProductDefinitionSection branch below writes
                // the model-version date from referenceDateTime instead.
                const eckit::DateTime& referenceDateTime = pt.labelDateTime;

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

                // Model-version date/time = ProductTime::referenceDateTime
                // (derived from date/time or year/month per §7.4).
                // TODO: Need to clarify with DGOV if this is reference or initialConditionsDateTime.
                const eckit::DateTime& dateTime = pt.referenceDateTime;

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
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_

#pragma once

#include <exception>
#include <iostream>
#include <string>


// Logging
#include "metkit/config/LibMetkit.h"

// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/reference-time/reference_time_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/hindcastDateTime.h"
#include "metkit/mars2grib/backend/deductions/referenceDateTime.h"
#include "metkit/mars2grib/backend/deductions/significanceOfReferenceTime.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchProductDefinitionTemplateNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
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

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, ReferenceTimeType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void ReferenceTimeOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                     OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::check;
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (referenceTimeApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Time] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(referenceTimeTypeName<Variant>()) << std::endl;


            // ===============================================================
            // Set significance of reference time
            if constexpr (Section == SecIdentificationSection) {

                // Significance of reference time
                deductions::SignificanceOfReferenceTime significanceOfReferenceTime =
                    deductions::significanceOfReferenceTime(mars, par);

                set_or_throw<long>(out, "significanceOfReferenceTime", static_cast<long>(significanceOfReferenceTime));

            }  // if constexpr ( Section == SectionType::IdentificationSection )


            // ===============================================================
            // Set date and time components for a standard forecast
            if constexpr ((Section == SecIdentificationSection) && (Variant == ReferenceTimeType::Standard)) {

                // Deduce date and time components from dateTime deduction
                auto dateTime = deductions::referenceDateTime(mars, par);

                // Set date and time components in output dictionary
                set_or_throw<long>(out, "year", dateTime.date().year());
                set_or_throw<long>(out, "month", dateTime.date().month());
                set_or_throw<long>(out, "day", dateTime.date().day());
                set_or_throw<long>(out, "hour", dateTime.time().hours());
                set_or_throw<long>(out, "minute", dateTime.time().minutes());
                set_or_throw<long>(out, "second", dateTime.time().seconds());

                return;

            }  // if constexpr ( ( Section == SectionType::IdentificationSection ) && (Variant == TimeType::Standard) )


            // ===============================================================
            // Set date and time components for a reforecast hindcast
            if constexpr ((Section == SecIdentificationSection) && (Variant == ReferenceTimeType::Reforecast)) {

                auto referenceDateTime = deductions::hindcastDateTime(mars, par);

                // Set date and time components in output dictionary
                set_or_throw<long>(out, "year", referenceDateTime.date().year());
                set_or_throw<long>(out, "month", referenceDateTime.date().month());
                set_or_throw<long>(out, "day", referenceDateTime.date().day());
                set_or_throw<long>(out, "hour", referenceDateTime.time().hours());
                set_or_throw<long>(out, "minute", referenceDateTime.time().minutes());
                set_or_throw<long>(out, "second", referenceDateTime.time().seconds());

                return;

            }  // if constexpr ( ( Section == SectionType::IdentificationSection ) && (Variant == TimeType::Reforecast)
               // )


            // ===============================================================
            // Set model time for reforecast hindcast
            if constexpr ((Section == SecProductDefinitionSection) && (Variant == ReferenceTimeType::Reforecast)) {

                // Checks on template number to ensure this is a reforecast product
                checks::matchProductDefinitionTemplateNumber_or_throw(opt, out, {60L, 61L});


                // Deduce date and time components from dateTime deduction
                auto dateTime = deductions::referenceDateTime(mars, par);

                // Set date and time components in output dictionary
                set_or_throw<long>(out, "YearOfModelVersion", dateTime.date().year());
                set_or_throw<long>(out, "MonthOfModelVersion", dateTime.date().month());
                set_or_throw<long>(out, "DayOfModelVersion", dateTime.date().day());
                set_or_throw<long>(out, "HourOfModelVersion", dateTime.time().hours());
                set_or_throw<long>(out, "MinuteOfModelVersion", dateTime.time().minutes());
                set_or_throw<long>(out, "SecondOfModelVersion", dateTime.time().seconds());

                return;


            }  // if constexpr ( (Section == SectionType::ProductDefinitionSection) && ( Variant == TimeType::Reforecast
               // ) )
        }
        catch (...) {

            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(referenceTimeName), std::string(referenceTimeTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `referenceTime` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( referenceTimeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(referenceTimeName), std::string(referenceTimeTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

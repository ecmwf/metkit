#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/statistics/statistics_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/marsStepInSeconds.h"
#include "metkit/mars2grib/backend/deductions/marsTimeSpanInSeconds.h"
#include "metkit/mars2grib/backend/deductions/numberOfTimeRanges.h"
#include "metkit/mars2grib/backend/deductions/statisticsDescriptor.h"
#include "metkit/mars2grib/backend/deductions/timeIncrementInSeconds.h"

// checks
#include "metkit/mars2grib/backend/checks/isStatisticsProductDefinitionTemplateNumber.h"

// Utils
#include "metkit/mars2grib/utils/timeUtils.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Sec, StatisticsType Variant>
constexpr bool statisticsApplicable() {
    return (Sec == SecProductDefinitionSection);
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Sec, StatisticsType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void StatisticsOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                  OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;


    if constexpr (statisticsApplicable<Stage, Sec, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Statistics] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Sec
                                     << ", Variant=" << std::string(statisticsTypeName<Variant>()) << std::endl;

            // =============================================================

            // Get number of timeranges
            long numberOfTimeRangesVal = deductions::numberOfTimeRanges(mars, par);

            if constexpr (Stage == StageAllocate) {

                // Set statistics type
                checks::isStatisticsProductDefinitionTemplateNumber_or_throw(opt, out);

                // Set number of time ranges
                set_or_throw<long>(out, "numberOfTimeRanges", numberOfTimeRangesVal);
            }

            if constexpr (Stage == StagePreset) {

                // Get the length of timestep in seconds
                std::optional<long> timeIncrementInSecondsOpt = deductions::timeIncrementInSeconds_opt(mars, par);

                // Set type of statistical processing
                set_or_throw<long>(out, "typeOfStatisticalProcessing", typeOfStatisticalProcessing<Variant>());
                set_or_throw<long>(out, "indicatorOfUnitOfTimeRange", static_cast<long>(utils::time::TimeUnit::Hour));
                set_or_throw<long>(out, "indicatorOfUnitForTimeRange", static_cast<long>(utils::time::TimeUnit::Hour));


                // HACK: handle special case for MUL-227
                if (numberOfTimeRangesVal == 1 && !timeIncrementInSecondsOpt.has_value()) {

                    set_or_throw<long>(out, "typeOfTimeIncrement", 2L);  // CodeTable 4.11
                    set_or_throw<long>(out, "indicatorOfUnitForTimeIncrement",
                                       static_cast<long>(utils::time::TimeUnit::Missing));
                    set_or_throw<long>(out, "timeIncrement", 0L);
                }
                else {

                    set_or_throw<long>(out, "typeOfTimeIncrement", 2L);  // CodeTable 4.11
                    set_or_throw<long>(out, "indicatorOfUnitForTimeIncrement",
                                       static_cast<long>(utils::time::TimeUnit::Second));
                    set_or_throw<long>(out, "timeIncrement", timeIncrementInSecondsOpt.value());

                    deductions::StatisticalProcessing statsDesc = deductions::getTimeDescriptorFromMars_orThrow(
                        mars, par, typeOfStatisticalProcessing<Variant>());

                    if (numberOfTimeRangesVal > 1) {
                        throw Mars2GribConceptException(
                            std::string(statisticsName), std::string(statisticsTypeName<Variant>()),
                            std::to_string(Stage), std::to_string(Sec),
                            "`statistics` concept with multiple time ranges not yet supported at preset stage", Here());
                    }
                }
            }

            if constexpr (Stage == StageRuntime) {


                long stepInHour     = deductions::marsStepInSeconds_or_throw(mars, par) / 3600;
                long timeSpanInHour = deductions::marsTimeSpanInSeconds_or_throw(mars, par) / 3600;

                // Get the length of timestep in seconds
                std::optional<long> timeIncrementInSecondsOpt = deductions::timeIncrementInSeconds_opt(mars, par);

                long tmp       = stepInHour - timeSpanInHour;
                long startStep = (tmp >= 0) ? tmp : 0;
                long endStep   = stepInHour;

                set_or_throw<long>(out, "startStep", startStep);
                set_or_throw<long>(out, "endStep", endStep);
                if (timeIncrementInSecondsOpt.has_value()) {
                    deductions::StatisticalProcessing statsDesc = deductions::getTimeDescriptorFromMars_orThrow(
                        mars, par, typeOfStatisticalProcessing<Variant>());
                };


                if (numberOfTimeRangesVal > 1) {
                    throw Mars2GribConceptException(
                        std::string(statisticsName), std::string(statisticsTypeName<Variant>()), std::to_string(Stage),
                        std::to_string(Sec),
                        "`statistics` concept with multiple time ranges not yet supported at preset stage", Here());
                }
            }
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(statisticsName), std::string(statisticsTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Sec), "Unable to set `statistics` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(statisticsName), std::string(statisticsTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Sec), "Concept called when not applicable...",
                                    Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts

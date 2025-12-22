#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/forecastTimeInSeconds.h"
#include "metkit/mars2grib/backend/deductions/numberOfTimeRanges.h"
#include "metkit/mars2grib/backend/deductions/timeSpanInSeconds.h"

// Utils
#include "metkit/mars2grib/backend/deductions/detail/timeUtils.h"
#include "metkit/mars2grib/backend/deductions/timeIncrementInSeconds.h"

// Exceptions
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

struct StatisticalProcessing {

    long numberOfTimeRanges = 0;

    std::vector<long> typeOfStatisticalProcessing;
    std::vector<long> typeOfTimeIncrement;
    std::vector<long> indicatorOfUnitForTimeRange;
    std::vector<long> lengthOfTimeRange;
    std::vector<long> indicatorOfUnitForTimeIncrement;
    std::vector<long> lengthOfTimeIncrement;
};

template <class MarsDict_t, class ParDict_t, class OptDict_t>
inline StatisticalProcessing getTimeDescriptorFromMars_orThrow(
    const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt,
    long outerStatOp  // typeOfStatisticalProcessing for inner loop
) {

    using metkit::mars2grib::backend::deductions::detail::parseStatType_or_throw;
    using metkit::mars2grib::backend::deductions::detail::Period;
    using metkit::mars2grib::backend::deductions::detail::previousMonthLengthHours;
    using metkit::mars2grib::backend::deductions::detail::StatOp;
    using metkit::mars2grib::backend::deductions::detail::StatTypeBlock;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        StatisticalProcessing out{};

        // ---------------------------------------------------------------------
        // Number of loops
        // ---------------------------------------------------------------------
        long numberOfTimeRangesVal = numberOfTimeRanges(mars, par);

        // Validate number of time ranges
        if (numberOfTimeRangesVal < 1 || numberOfTimeRangesVal > 3) {
            throw Mars2GribDeductionException("Unexpected number of time loops", Here());
        }

        // Initialize output structure
        out.numberOfTimeRanges = numberOfTimeRangesVal;
        out.typeOfStatisticalProcessing.reserve(numberOfTimeRangesVal);
        out.typeOfTimeIncrement.reserve(numberOfTimeRangesVal);
        out.indicatorOfUnitForTimeRange.reserve(numberOfTimeRangesVal);
        out.lengthOfTimeRange.reserve(numberOfTimeRangesVal);
        out.indicatorOfUnitForTimeIncrement.reserve(numberOfTimeRangesVal);
        out.lengthOfTimeIncrement.reserve(numberOfTimeRangesVal);

        // ---------------------------------------------------------------------
        // Parse stattype blocks (outer loops)
        // ---------------------------------------------------------------------

        std::vector<StatTypeBlock> blocks;
        if (numberOfTimeRangesVal > 1) {
            // Get the stattype from mars
            std::string statTypeVal = get_or_throw<std::string>(mars, "stattype");

            // Parse stattype
            blocks = parseStatType_or_throw(statTypeVal);
        }

        // ---------------------------------------------------------------------
        // End date (needed for monthly length)
        // ---------------------------------------------------------------------
        eckit::DateTime forecastTime = resolve_ForecastTimeInSeconds_or_throw(mars, par, opt);

        long endYear  = forecastTime.date().year();
        long endMonth = forecastTime.date().month();

        const long timeStepSeconds = timeIncrementInSeconds_or_throw(mars, par);

        const long timeSpanInSeconds = resolve_TimeSpanInSeconds_or_throw(mars, par, opt);

        if (timeSpanInSeconds % 3600 != 0) {
            throw Mars2GribDeductionException("`timespan` must be multiple of 3600 seconds", Here());
        }
        const long timeSpanHours = timeSpanInSeconds / 3600;


        // ---------------------------------------------------------------------
        // Fill SoA (exactly like Fortran)
        // ---------------------------------------------------------------------

        for (long i = 0; i < numberOfTimeRangesVal; ++i) {

            const bool isInnerLoop = (i == numberOfTimeRangesVal - 1);

            // Common fields (hard-coded in Fortran)
            out.typeOfTimeIncrement[i]             = 2;   // multIO fixed value
            out.indicatorOfUnitForTimeRange[i]     = 1;   // hours
            out.indicatorOfUnitForTimeIncrement[i] = 13;  // seconds

            if (isInnerLoop) {
                // -------------------------------------------------------------
                // Inner loop (timespan)
                // -------------------------------------------------------------
                out.typeOfStatisticalProcessing[i] = outerStatOp;
                out.lengthOfTimeRange[i]           = timeSpanHours;
                out.lengthOfTimeIncrement[i]       = timeStepSeconds;
            }
            else {
                // -------------------------------------------------------------
                // Outer loop(s) from stattype
                // -------------------------------------------------------------
                const auto& blk = blocks[i];

                out.typeOfStatisticalProcessing[i] = static_cast<long>(blk.op);

                // lengthOfTimeRange
                if (blk.period == Period::Daily) {
                    out.lengthOfTimeRange[i] = 24;
                }
                else if (blk.period == Period::Monthly) {
                    out.lengthOfTimeRange[i] = previousMonthLengthHours(endYear, endMonth);
                }
                else {
                    throw std::runtime_error("Unsupported period");
                }

                // lengthOfTimeIncrement
                if (i == numberOfTimeRangesVal - 2) {
                    // Next loop is inner → use timespan
                    out.lengthOfTimeIncrement[i] = timeSpanHours * 3600;
                }
                else {
                    // Next loop is another stattype block
                    const auto& nextBlk = blocks[i + 1];

                    if (nextBlk.period == Period::Daily) {
                        out.lengthOfTimeIncrement[i] = 24 * 3600;
                    }
                    else if (nextBlk.period == Period::Monthly) {
                        out.lengthOfTimeIncrement[i] = previousMonthLengthHours(endYear, endMonth) * 3600;
                    }
                    else {
                        throw Mars2GribDeductionException("Unsupported next period", Here());
                    }
                }
            }

            return out;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to compute statistics descriptor from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

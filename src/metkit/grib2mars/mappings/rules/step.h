#pragma once

#include <algorithm>
#include <optional>
#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

bool isStrikeProbability(long paramId) {

    return (paramId == 131074 || paramId == 131075 || paramId == 131076 || paramId == 131077 || paramId == 131089 ||
            paramId == 131090 || paramId == 131091);
}

/// Seconds per unit of time of code table 4.4, for the units of fixed length (month, year, ... have none)
inline std::optional<long> secondsPerUnitOfTime(long indicatorOfUnitOfTime) {
    switch (indicatorOfUnitOfTime) {
        case 0:  // minute
            return 60;
        case 1:  // hour
            return 3600;
        case 2:  // day
            return 86400;
        case 10:  // 3 hours
            return 3 * 3600;
        case 11:  // 6 hours
            return 6 * 3600;
        case 12:  // 12 hours
            return 12 * 3600;
        case 13:  // second
            return 1;
        default:
            return std::nullopt;
    }
}

/// Time increment of the statistical processing in seconds, from the GRIB2 header: the innermost time range that has
/// one. None when the message has no time ranges, or every increment is missing or in a unit without a fixed length.
inline std::optional<long> timeIncrementInSecondsFromHeader(const metkit::codes::CodesHandle& grib) {
    if (!grib.has("timeIncrement") || !grib.has("indicatorOfUnitForTimeIncrement")) {
        return std::nullopt;
    }
    const auto increments = grib.getLongArray("timeIncrement");
    const auto units      = grib.getLongArray("indicatorOfUnitForTimeIncrement");
    for (std::size_t i = std::min(increments.size(), units.size()); i-- > 0;) {
        const auto secondsPerUnit = secondsPerUnitOfTime(units[i]);
        if (secondsPerUnit && increments[i] > 0) {
            return increments[i] * *secondsPerUnit;
        }
    }
    return std::nullopt;
}

template <class MarsDict, class MiscDict, class OptDict_t>
void extractStep(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                 const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::get_or_throw;
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        const bool isInstant             = grib.getString("stepType") == "instant";
        const bool hasTimespan           = grib.has("timespan");
        const bool isStandardisedAnomaly = grib.getString("stepType") == "stdanom" &&
                                           get_or_throw<bool>(opts, "tryFixBadInput_StandardisedAnomalyAsInstant");


        if (isInstant || hasTimespan || isStandardisedAnomaly) {
            if (!grib.has("endStep")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `endStep` required to extract MARS keyword `" + keyword + "`", Here());
            }

            const long endStep = grib.getLong("endStep");

            set_or_throw<long>(mars, keyword, endStep);
        }
        else {
            if (!grib.has("startStep")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `startStep` required to extract MARS keyword `" + keyword + "`", Here());
            }

            if (!grib.has("endStep")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `endStep` required to extract MARS keyword `" + keyword + "`", Here());
            }

            const long startStep        = grib.getLong("startStep");
            const long endStep          = grib.getLong("endStep");
            const std::string stepRange = std::to_string(startStep) + "-" + std::to_string(endStep);

            if (isStrikeProbability(grib.getLong("paramId")) &&
                get_or_throw<bool>(opts, "tryFixBadInput_RemoveStepRangeForStrikeProbability")) {
                set_or_throw<long>(mars, keyword, startStep);
                return;
            }
            set_or_throw<std::string>(mars, keyword, stepRange);
        }

        // `timeIncrement` is in the unit of `indicatorOfUnitForTimeIncrement`, not necessarily seconds
        if (const auto timeIncrementInSeconds = timeIncrementInSecondsFromHeader(grib)) {
            misc.set("timeIncrementInSeconds", *timeIncrementInSeconds);
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
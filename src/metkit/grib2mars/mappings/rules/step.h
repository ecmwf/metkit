#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

bool isStrikeProbability( long paramId ){

    return (paramId == 131074 || paramId == 131075 || paramId == 131076 || paramId == 131077 ||
            paramId == 131089 || paramId == 131090 || paramId == 131091);

}

template <class MarsDict, class MiscDict, class OptDict_t>
void extractStep(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                 const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::dict_traits::get_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        const bool isInstant   = grib.getString("stepType") == "instant";
        const bool hasTimespan = grib.has("timespan");
        const bool isStandardisedAnomaly = grib.getString("stepType") == "stdanom" && get_or_throw<bool>(opts, "tryFixBadInput_StandardisedAnomalyAsInstant");


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

            if ( isStrikeProbability(grib.getLong("paramId")) &&
                get_or_throw<bool>( opts, "tryFixBadInput_RemoveStepRangeForStrikeProbability" ) ) {
                set_or_throw<long>(mars, keyword, startStep);
                return;
            }
            set_or_throw<std::string>(mars, keyword, stepRange);
        }

        if (grib.has("timeIncrement")) {
            const long timeIncrement = grib.getLong("timeIncrement");
            if (timeIncrement > 0) {
                misc.set("timeIncrementInSeconds", timeIncrement);
            }
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
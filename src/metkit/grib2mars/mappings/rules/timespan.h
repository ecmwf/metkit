#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

namespace detail {
template <class MarsDict, class OptDict_t>
void tryFixBadInput_ZeroAccumulation(const metkit::codes::CodesHandle& grib, MarsDict& mars,
                                     const std::string& stepType, long startStep, long endStep, const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::get_or_throw;
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        long marsParamId = get_or_throw<long>(mars, "paramId");

        // This fix an inconsistency in the current grib fields in production
        if (marsParamId == 162110 && stepType == "accum") {
            set_or_throw<std::string>(mars, "timespan", "fs");
        }
        else {
            throw Grib2MarsGenericException(
                "Cannot fix bad input for zero accumulation for MARS keyword `timespan` with param=" +
                    std::to_string(marsParamId) + ", stepType=`" + stepType +
                    "`, startStep=" + std::to_string(startStep) + ", endStep=" + std::to_string(endStep),
                Here());
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException(
            "Failed to fix bad input for zero accumulation while extracting MARS keyword `timespan`", Here()));
    }
}
}  // namespace detail


template <class MarsDict, class MiscDict, class OptDict_t>
void extractTimespan(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                     const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::get_or_throw;
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        (void)misc;

        if (!grib.has("stepType")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `stepType` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const std::string stepType = grib.getString("stepType");

        if (stepType == "instant") {
            set_or_throw<std::string>(mars, keyword, "none");
            return;
        }

        if (!grib.has("startStep")) {
            throw Grib2MarsGenericException("Missing GRIB key `startStep` required to extract MARS keyword `" +
                                                keyword + "` for statistical stepType `" + stepType + "`",
                                            Here());
        }

        if (!grib.has("endStep")) {
            throw Grib2MarsGenericException("Missing GRIB key `endStep` required to extract MARS keyword `" + keyword +
                                                "` for statistical stepType `" + stepType + "`",
                                            Here());
        }

        const long startStep = grib.getLong("startStep");
        const long endStep   = grib.getLong("endStep");

        const long timespan = endStep - startStep;

        if (timespan < 0) {
            throw Grib2MarsGenericException(
                "Invalid statistical window while extracting MARS keyword `" + keyword + "`: stepType=`" + stepType +
                    "`, startStep=" + std::to_string(startStep) + ", endStep=" + std::to_string(endStep) +
                    ", timespan=" + std::to_string(timespan),
                Here());
        }

        if (timespan == 0 && get_or_throw<bool>(opts, "tryFixBadInput_ZeroAccumulation")) {
            detail::tryFixBadInput_ZeroAccumulation(grib, mars, stepType, startStep, endStep, opts);
            return;
        }
        else {
            throw Grib2MarsGenericException(
                "Invalid statistical window while extracting MARS keyword `" + keyword + "`: stepType=`" + stepType +
                    "`, startStep=" + std::to_string(startStep) + ", endStep=" + std::to_string(endStep) +
                    ", timespan=" + std::to_string(timespan),
                Here());
        }

        set_or_throw<long>(mars, keyword, timespan);
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
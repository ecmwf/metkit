#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>


#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
std::optional<long> timeIncrementInSeconds_opt(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.expver
        auto lengthOfTimeStepInSeconds_opt = get_opt<long>(par, "timeIncrementInSeconds");

        // TODO MIVAL: Validate (if present needs to be > 0)
        if (lengthOfTimeStepInSeconds_opt.has_value()) {
            if (lengthOfTimeStepInSeconds_opt.value() < 0) {
                throw Mars2GribDeductionException("`timeIncrementInSeconds` must be > 0 if present", Here());
            }
            else if (lengthOfTimeStepInSeconds_opt.value() == 0) {
                lengthOfTimeStepInSeconds_opt = std::nullopt;
            }
        }

        return lengthOfTimeStepInSeconds_opt;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `timeIncrementInSeconds` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


template <class MarsDict_t, class ParDict_t>
long timeIncrementInSeconds_or_throw(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        std::optional<long> timeIncrementInSecondsOpt = timeIncrementInSeconds_opt(mars, par);

        if (timeIncrementInSecondsOpt.has_value()) {
            return timeIncrementInSecondsOpt.value();
        }
        else {
            throw Mars2GribDeductionException("`timeIncrementInSeconds` is not defined in Mars/Par dictionary", Here());
        }
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `timeIncrementInSeconds` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};


}  // namespace metkit::mars2grib::backend::deductions

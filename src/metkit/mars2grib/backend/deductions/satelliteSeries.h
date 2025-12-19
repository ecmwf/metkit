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
long satelliteSeries_or_throw(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the par.satelliteseries
        auto marsSatelliteSeriesVal = get_or_throw<long>(par, "satelliteSeries");

        // TODO MIVAL: Validate

        return marsSatelliteSeriesVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `satelliteSeries` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

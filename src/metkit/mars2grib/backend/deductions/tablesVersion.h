#pragma once

#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
long tablesVersion(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the the latest tables version from eccodes.
        // Unfortunately there is no other way to get it from eccodes other than loading a sample
        // use the get functionality to read the latest tables version (which is not in the sample itself)
        // And then destory the sample handle
        return metkit::codes::codesHandleFromSample("GRIB2")->getLong("tablesVersionLatest");
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Could not deduce `tablesVersion` from GRIB2 sample", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

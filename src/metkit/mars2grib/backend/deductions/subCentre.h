#pragma once

#include <optional>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t>
long sub_centre(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    // Stupid deduction rule, but it is what it is for now
    try {

        return get_opt<long>(par, "subCentre").value_or(0L);
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `subCentre` as string from Par dictionary", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

#pragma once

#include <exception>
#include <optional>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::deductions {

// grib2 section 2.36 for analysis fields. octets 19-20
template <class MarsDict_t, class ParDict_t>
std::optional<long> lengthOfTimeWindow_opt(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Stupid deduction rule, but it is what it is for now
        return get_opt<long>(par, "lengthOfTimeWindow");
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `anoffset` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::deductions

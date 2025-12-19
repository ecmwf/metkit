#pragma once

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"


// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// exception and logging
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

inline long to_long_or_throw(std::string_view s) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    long v{};
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v, 10);

    if (ec == std::errc::invalid_argument)
        throw Mars2GribDeductionException("invalid long: '" + std::string(s) + "'", Here());
    if (ec == std::errc::result_out_of_range)
        throw Mars2GribDeductionException("long out of range: '" + std::string(s) + "'", Here());
    if (ptr != s.data() + s.size())
        throw Mars2GribDeductionException("invalid long (trailing chars): '" + std::string(s) + "'", Here());
    return v;
}

template <class MarsDict_t, class ParDict_t>
long paramId(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // // Lookup origin from the mars dictionary
        // auto paramId = get_or_throw<std::string>( mars, "param" );

        // // TODO MIVAL: validate paramId format

        // // Return validated origin
        // return to_long_or_throw( paramId );

        // TODO : Implement proper string -> long parsing
        return get_or_throw<long>(mars, "param");
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `param` as string from Mars dictionary", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::deductions

/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
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

/**
 * @brief Resolve the wave direction number from the MARS dictionary.
 *
 * This deduction retrieves the value associated with the key `direction`
 * from the MARS dictionary (`mars`). The value is expected to be convertible
 * to a `long` and is treated as mandatory.
 *
 * The resolved value typically represents the wave direction number or
 * direction index used in wave spectral processing and GRIB encoding.
 *
 * If the key is missing or the value cannot be converted to the expected
 * type, a domain-specific exception is thrown. Any underlying error is
 * propagated using nested exceptions to preserve full diagnostic context.
 *
 * The resolved value is logged for diagnostic and traceability purposes.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `direction`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the wave direction number is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The wave direction number resolved from the MARS dictionary, returned
 *   as a `long`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `direction` is not present in the MARS dictionary,
 *   - the associated value cannot be converted to `long`,
 *   - any unexpected error occurs during dictionary access.
 *
 * @note
 *   This deduction assumes that the wave direction number is explicitly
 *   provided by the MARS dictionary and does not attempt any inference
 *   or defaulting.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested exception
 *   propagation to ensure that error provenance is preserved across API
 *   boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_WaveDirectionNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.dir
        auto waveDirectionNumber = get_or_throw<long>(mars, "direction");

        // Logging of the paramId
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "waveDirectionNumber: looked up from Mars dictionary with value: ";
            logMsg += std::to_string(waveDirectionNumber);
            return logMsg;
        }());

        return waveDirectionNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `dir` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

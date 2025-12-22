#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the MARS data stream from the MARS dictionary.
 *
 * This deduction retrieves the value associated with the key `stream`
 * from the MARS dictionary (`mars`). The value is expected to be a
 * string and is treated as mandatory.
 *
 * The resolved value typically represents the MARS data stream
 * identifier (e.g. operational stream, ensemble stream, or other
 * production streams) as defined by upstream MARS conventions.
 * The semantic interpretation of the stream value is not performed
 * by this deduction.
 *
 * The resolved value is logged for diagnostic and traceability
 * purposes.
 *
 * If the key is missing or the value cannot be converted to the
 * expected type, a domain-specific exception is thrown. Any
 * underlying error is propagated using nested exceptions to
 * preserve full diagnostic context.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `stream`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the data stream identifier is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The data stream identifier resolved from the MARS dictionary,
 *   returned as a `std::string`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `stream` is not present in the MARS dictionary,
 *   - the associated value cannot be converted to `std::string`,
 *   - any unexpected error occurs during dictionary access.
 *
 * @note
 *   This deduction assumes that the MARS data stream is explicitly
 *   provided by the MARS dictionary and does not attempt any
 *   inference, defaulting, or validation of the returned value.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested
 *   exception propagation to ensure that error provenance is
 *   preserved across API boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::string resolve_Stream_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.stream
        std::string marsStreamVal = get_or_throw<std::string>(mars, "stream");

        // Logging of the channel
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "stream: deduced from mars dictionary with value: " + marsStreamVal;
            return logMsg;
        }());

        return marsStreamVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `stream` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

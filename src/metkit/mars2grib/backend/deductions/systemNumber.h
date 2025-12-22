#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the wave system identifier from the MARS dictionary.
 *
 * This deduction retrieves the value associated with the key `system`
 * from the MARS dictionary (`mars`). The value is expected to be convertible
 * to a `long` and is treated as mandatory.
 *
 * The resolved value typically represents a system identifier used to
 * distinguish different wave systems or model configurations within the
 * wave spectral framework. The numerical meaning of the identifier is not
 * interpreted by this deduction and is assumed to be defined upstream.
 *
 * If the key is missing or the value cannot be converted to the expected
 * type, a domain-specific exception is thrown. Any underlying error is
 * propagated using nested exceptions to preserve full diagnostic context.
 *
 * The resolved value is logged for diagnostic and traceability purposes.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary, expected to contain the key `system`.
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary (unused by this deduction).
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary from which the system identifier is retrieved.
 *
 * @param[in] par
 *   Parameter dictionary (unused).
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The system identifier resolved from the MARS dictionary, returned
 *   as a `long`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `system` is not present in the MARS dictionary,
 *   - the associated value cannot be converted to `long`,
 *   - any unexpected error occurs during dictionary access.
 *
 * @note
 *   This deduction assumes that the system identifier is explicitly
 *   provided by the MARS dictionary and does not attempt any inference
 *   or defaulting.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested exception
 *   propagation to ensure that error provenance is preserved across API
 *   boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SystemNumber_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.system
        auto systemNumber = get_or_throw<long>(mars, "system");

        // Logging of the systemNumber
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "systemNumber: looked up from Mars dictionary with value: ";
            logMsg += std::to_string(systemNumber);
            return logMsg;
        }());

        return systemNumber;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `system` from Mars dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions

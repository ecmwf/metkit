#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `satelliteSeries` key.
 *
 * This deduction determines the value of the GRIB `satelliteSeries` key,
 * which identifies the satellite series associated with the product.
 *
 * @details
 * The value of `satelliteSeries` **cannot be inferred** from the MARS
 * request and must be provided explicitly via the parameter dictionary
 * (`par`).
 *
 * No defaulting or fallback behavior is implemented. The absence of this
 * key is considered a deduction error.
 *
 * @important
 * This function is the **single authoritative deduction** for
 * `satelliteSeries`.
 * The encoder must not rely on any pre-existing GRIB header state or
 * implicit ecCodes behavior for this key.
 *
 * @tparam MarsDict_t Type of the MARS dictionary (unused)
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary (unused)
 * @param[in] par  Parameter dictionary; must contain `satelliteSeries`
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The satellite series identifier to be encoded in the GRIB message.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - `satelliteSeries` is missing from the parameter dictionary
 *         - the value cannot be retrieved as a `long`
 *         - any unexpected error occurs during deduction
 *
 * @note
 * - This deduction is fully deterministic.
 * - No validation against GRIB code tables is currently performed.
 *
 * @todo [owner: mival,dgov][scope: deduction][reason: correctness][prio: medium]
 * - Validate `satelliteSeries` against the corresponding GRIB code table.
 * - Clarify whether this key should become inferable from MARS metadata
 *   for satellite-based products.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SatelliteSeries_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the par.satelliteseries
        long marsSatelliteSeriesVal = get_or_throw<long>(par, "satelliteSeries");

        // Logging of the par::satelliteSeries
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "satelliteSeries: looked up from Par dictionary with value: ";
            logMsg += std::to_string(marsSatelliteSeriesVal);
            return logMsg;
        }());

        // Return the value
        return marsSatelliteSeriesVal;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to resolve `satelliteSeries` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
